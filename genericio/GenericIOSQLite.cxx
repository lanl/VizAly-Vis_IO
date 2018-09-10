/*
 *                    Copyright (C) 2015, UChicago Argonne, LLC
 *                               All Rights Reserved
 *
 *                               Generic IO (ANL-15-066)
 *                     Hal Finkel, Argonne National Laboratory
 *                  Jon Woodring, Los Alamos National Laboratory
 *                 Pascal Grosset, Los Alamos National Laboratory
 *
 *                              OPEN SOURCE LICENSE
 *
 * Under the terms of Contract No. DE-AC02-06CH11357 with UChicago Argonne,
 * LLC, the U.S. Government retains certain rights in this software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the names of UChicago Argonne, LLC or the Department of Energy
 *      nor the names of its contributors may be used to endorse or promote
 *      products derived from this software without specific prior written
 *      permission.
 *
 * *****************************************************************************
 *
 *                                  DISCLAIMER
 * THE SOFTWARE IS SUPPLIED “AS IS” WITHOUT WARRANTY OF ANY KIND.  NEITHER THE
 * UNTED STATES GOVERNMENT, NOR THE UNITED STATES DEPARTMENT OF ENERGY, NOR
 * UCHICAGO ARGONNE, LLC, NOR ANY OF THEIR EMPLOYEES, MAKES ANY WARRANTY,
 * EXPRESS OR IMPLIED, OR ASSUMES ANY LEGAL LIABILITY OR RESPONSIBILITY FOR THE
 * ACCURACY, COMPLETENESS, OR USEFULNESS OF ANY INFORMATION, DATA, APPARATUS,
 * PRODUCT, OR PROCESS DISCLOSED, OR REPRESENTS THAT ITS USE WOULD NOT INFRINGE
 * PRIVATELY OWNED RIGHTS.
 *
 * *****************************************************************************
 */

#include "GenericIO.h"
#include <cstring>
#include <vector>
#include <string>
#include <limits>
#include <sstream>
#include <sqlite3ext.h>
#include <math.h>
SQLITE_EXTENSION_INIT1

#define GIO_EQUAL_CONSTRAINT 'E'
#define GIO_GRTEQ_CONSTRAINT 'G'
#define GIO_GREAT_CONSTRAINT 'g'
#define GIO_LESEQ_CONSTRAINT 'L'
#define GIO_LESST_CONSTRAINT 'l'

#define GIO_RANK_INDEX 'r'
#define GIO_PERCENT_INDEX 'p'
#define GIO_X_INDEX 'x'
#define GIO_Y_INDEX 'y'
#define GIO_Z_INDEX 'z'

#define GIO_INDEX_SEP ';'

using namespace std;
using namespace gio;

class PrinterBase
{
  public:
    virtual ~PrinterBase() {}
    virtual void print(sqlite3_context *cxt, size_t i) = 0;
};

// conversion routine from GIO to SQLite
template <class T>
class Printer : public PrinterBase
{
  public:
    Printer(GenericIO &G, size_t MNE, const string &N)
        : Data(MNE + G.requestedExtraSpace() / sizeof(T))
    {
        // I'm not sure what this does - Jon
        // I *think* it gets a pointer to the data per rank
        // with equal array sizes for Data
        G.addVariable(N, Data, true);
    }

    virtual void print(sqlite3_context *cxt, size_t i)
    {
        T d = Data[i];
        if (!numeric_limits<T>::is_integer)
        {
            sqlite3_result_double(cxt, double(d));
        }
        else
        {
            if (sizeof(T) > 4)
            {
                sqlite3_result_int64(cxt, (sqlite3_int64) d);
            }
            else
            {
                sqlite3_result_int(cxt, int(d));
            }
        }
    }

  protected:
    vector<T> Data;
};

// try creating a conversion routine from GIO to SQLite
template <typename T>
PrinterBase *addPrinter(GenericIO::VariableInfo &V,
                        GenericIO &GIO, size_t MNE)
{
    if (sizeof(T) != V.Size)
    {
        return 0;
    }

    if (V.IsFloat == numeric_limits<T>::is_integer)
    {
        return 0;
    }

    if (V.IsSigned != numeric_limits<T>::is_signed)
    {
        return 0;
    }

    return new Printer<T>(GIO, MNE, V.Name);
}

struct OctreeNode
{
    double xMin;
    double xMax;
    double yMin;
    double yMax;
    double zMin;
    double zMax;
    size_t rows;
    size_t index;
};

typedef struct OctreeNode OctreeNode;


// See: Michael Owens. Query Anything with SQLite.
// Dr. Dobb's Journal. Nov. 2007.
// http://www.drdobbs.com/database/query-anything-with-sqlite/202802959
// See: http://www.sqlite.org/c3ref/load_extension.html
// See: http://www.sqlite.org/vtab.html

// the virtual table (representation of GIO as a table)
struct gio_vtab
{
    gio_vtab(const string &FN) : GIO(FN), FileName(FN)
    {
        memset(&vtab, 0, sizeof(sqlite3_vtab));

        GIO.openAndReadHeader(GenericIO::MismatchAllowed);
    }

    sqlite3_vtab          vtab;
    GenericIO             GIO;
    string                FileName;

    bool                         HasOctree;
    vector< vector<OctreeNode> > OctreeNodes;
    int                          RankColumnIndex;
    int                          PercentColumnIndex;
};

// the cursor (tracks where we are in GIO)
struct gio_cursor
{
    gio_cursor(const string &FN, gio_vtab* vtab) : GIO(FN)
    {
        memset(&cursor, 0, sizeof(sqlite3_vtab_cursor));

        GIO.openAndReadHeader(GenericIO::MismatchAllowed);

        size_t MaxNumElems = GIO.readNumElems(0);
        int NR = GIO.readNRanks();
        for (int i = 1; i < NR; ++i)
        {
            size_t NElem = GIO.readNumElems(i);
            MaxNumElems = max(MaxNumElems, NElem);
        }

        // create conversion routines from GIO to SQLite
        vector<GenericIO::VariableInfo> VI;
        GIO.getVariableInfo(VI);

        for (size_t i = 0; i < VI.size(); ++i)
        {
            PrinterBase *P = 0;

            // try creating conversion routines in this precedence
  #define ADD_PRINTER(T) \
      if (!P) P = addPrinter<T>(VI[i], GIO, MaxNumElems)
            ADD_PRINTER(float);
            ADD_PRINTER(double);
            ADD_PRINTER(unsigned char);
            ADD_PRINTER(signed char);
            ADD_PRINTER(int16_t);
            ADD_PRINTER(uint16_t);
            ADD_PRINTER(int32_t);
            ADD_PRINTER(uint32_t);
            ADD_PRINTER(int64_t);
            ADD_PRINTER(uint64_t);
  #undef ADD_PRINTER

            // add it
            Printers.push_back(P);
        }

        this->HasOctree = vtab->HasOctree;
        this->OctreeNodes = &(vtab->OctreeNodes);
        this->RankColumnIndex = vtab->RankColumnIndex;
        this->PercentColumnIndex = vtab->PercentColumnIndex;
    }

    sqlite3_vtab_cursor   cursor;
    GenericIO             GIO;
    vector<PrinterBase *> Printers;

    size_t                 Rank;
    size_t                 Leaf;
    size_t                 Index;
    size_t                 Count;
    size_t                 Offset;
    size_t                 Remain;
    uint64_t               RowId;
    vector<bool>           RankMask;
    vector< vector<bool> > OctreeMask;
    double                 PercentageStart;
    double                 PercentageEnd;

    bool                          HasOctree;
    vector< vector<OctreeNode> >* OctreeNodes;
    int                           RankColumnIndex;
    int                           PercentColumnIndex;
};

static
int gio_connect(sqlite3* db, void *, int argc, const char *const *argv,
                sqlite3_vtab **ppVTab, char **pzErr)
{
    if (argc < 4)
    {
        *pzErr = sqlite3_mprintf("%s", "No input file specified!");
        return SQLITE_ERROR;
    }

    *pzErr = 0;
    try
    {
        // create our virtual table
        string FileName(argv[3]);
        if (FileName.length () < 3)
        {
            *pzErr = sqlite3_mprintf("%s", "File name string incorrectly formatted!");
            return SQLITE_ERROR;
        }
        else if (!(FileName[0] == '\"' && FileName[FileName.length() - 1] == '\"') &&
                 !(FileName[0] == '\'' && FileName[FileName.length() - 1] == '\''))
        {
            *pzErr = sqlite3_mprintf("%s", "File name string incorrectly formatted!");
            return SQLITE_ERROR;
        }

        FileName = FileName.substr(1, FileName.length() - 2);
        gio_vtab *tab = new gio_vtab(FileName);

        // check for the existence of the octree file
        tab->HasOctree = false;


       
        if (tab->GIO.isOctree())
        {
            tab->HasOctree = true;
            GIOOctree sqliteOctreeData = tab->GIO.getOctree();

            int numRanks = tab->GIO.readNRanks();
            int numOctreeLeaves = sqliteOctreeData.numEntries;

            // add as many ranks as we need
            for (int i = 0; i < numRanks; i++)
                tab->OctreeNodes.push_back(vector<OctreeNode>());
            

            // we don't put the octree leaves into an explicit
            // tree. we just scan the bounding boxes of the leaves.
            // this ought to be pretty fast because there isn't
            // many leaves, as in most cases. if performance
            // is an issue, we can add an actual search hierarchy
            // for the leaves in the future
            for (int i=0; i<numOctreeLeaves; i++)
            {
                size_t rank;

                OctreeNode node;

                node.xMin = sqliteOctreeData.rows[i].minX;
                node.xMax = sqliteOctreeData.rows[i].maxX;
                node.yMin = sqliteOctreeData.rows[i].minY;
                node.yMax = sqliteOctreeData.rows[i].maxY;
                node.zMin = sqliteOctreeData.rows[i].minZ;
                node.zMax = sqliteOctreeData.rows[i].maxZ;
                node.rows = sqliteOctreeData.rows[i].numParticles;
                rank = sqliteOctreeData.rows[i].partitionLocation;
                node.index = sqliteOctreeData.rows[i].blockID;

                tab->OctreeNodes[rank].push_back(node);
            }
                
        }

        // create the schema for SQLite
        vector<GenericIO::VariableInfo> VI;
        tab->GIO.getVariableInfo(VI);

        string Schema = "CREATE TABLE x(";
        for (size_t i = 0; i < VI.size(); ++i)
        {
            // rename index to index_
            if (VI[i].Name == "index")
            {
                Schema += string("index_") + (VI[i].IsFloat ? " REAL" : " INTEGER") += ", ";
            }
            else
            {
                Schema += VI[i].Name + (VI[i].IsFloat ? " REAL" : " INTEGER") += ", ";
            }
        }

        // _rank is the rank data from GIO
        //
        // _percent is a floating point between 0.0 to 1.0
        // representing the index of a row (to restrict the amount of
        // data returned by a query, i.e., "where _percent < 0.5"
        // will only return 50% of the data)
        Schema += "_rank INTEGER HIDDEN, _percent REAL HIDDEN)";
        tab->RankColumnIndex = VI.size();
        tab->PercentColumnIndex = tab->RankColumnIndex + 1;

        //    printf("Schema: %s \n", Schema.c_str());

        // check to see if our schema was valid
        if (sqlite3_declare_vtab(db, Schema.c_str()) != SQLITE_OK)
        {
            *pzErr = sqlite3_mprintf("Could not declare schema: %s", Schema.c_str());
            delete tab;
            return SQLITE_ERROR;
        }

        *ppVTab = &tab->vtab;
    }
    catch (exception &e)
    {
        cerr << e.what() << "\n";

        *pzErr = sqlite3_mprintf("%s", e.what());
        return SQLITE_ERROR;
    }

    return SQLITE_OK;
}

static
int gio_create(sqlite3* db, void *pAux, int argc, const char *const *argv,
               sqlite3_vtab **ppVTab, char **pzErr)
{
    return gio_connect(db, pAux, argc, argv, ppVTab, pzErr);
}

static
int gio_disconnect(sqlite3_vtab *pVTab)
{
    gio_vtab *tab = (gio_vtab *) pVTab;

    delete tab;
    return SQLITE_OK;
}

static
int gio_destroy(sqlite3_vtab *pVTab)
{
    return gio_disconnect(pVTab);
}


static
int gio_best_index(sqlite3_vtab *pVTab, sqlite3_index_info *pInfo)
{
    gio_vtab *tab = (gio_vtab *) pVTab;

    int IndexIndex = 0;
    stringstream IndexName;

    vector<GenericIO::VariableInfo> VI;
    tab->GIO.getVariableInfo(VI);

    // go through the constraints and see if we can do them
    for (int i = 0; i < pInfo->nConstraint; ++i)
    {
        if (!pInfo->aConstraint[i].usable)
        {
            continue;
        }

        // indexing _rank
        char indexType = 0;
        if (pInfo->aConstraint[i].iColumn == tab->RankColumnIndex)
        {
            indexType = GIO_RANK_INDEX;
        }
        // indexing _percent
        else if (pInfo->aConstraint[i].iColumn == tab->PercentColumnIndex)
        {
            indexType = GIO_PERCENT_INDEX;
        }
        // indexing physical coordinates
        else if (tab->HasOctree)
        {
            int column = pInfo->aConstraint[i].iColumn;

            if (column >= 0 && column < VI.size())
            {
                // check to see if we have a physical coordinate
                if (VI[column].IsPhysCoordX)
                {
                    indexType = GIO_X_INDEX;
                }
                else if (VI[column].IsPhysCoordY)
                {
                    indexType = GIO_Y_INDEX;
                }
                else if (VI[column].IsPhysCoordZ)
                {
                    indexType = GIO_Z_INDEX;
                }
                // can't do it, skip
                else
                {
                    continue;
                }
            }
            // can't do it, skip
            else
            {
                continue;
            }
        }
        // can't do it, skip
        else
        {
            continue;
        }

        // create an op code for ourselves in use in xFilter
        char opcode = 0;
        switch (pInfo->aConstraint[i].op)
        {
        case SQLITE_INDEX_CONSTRAINT_EQ:
            opcode = GIO_EQUAL_CONSTRAINT;
            break;
        case SQLITE_INDEX_CONSTRAINT_GT:
            opcode = GIO_GREAT_CONSTRAINT;
            break;
        case SQLITE_INDEX_CONSTRAINT_LT:
            opcode = GIO_LESST_CONSTRAINT;
            break;
        case SQLITE_INDEX_CONSTRAINT_GE:
            opcode = GIO_GRTEQ_CONSTRAINT;
            break;
        case SQLITE_INDEX_CONSTRAINT_LE:
            opcode = GIO_LESEQ_CONSTRAINT;
            break;
        // skip if it's one we can't handle
        default:
            continue;
        }

        // create the index information to pass to ourselves in xFilter
        // pass the column, type of index, and direction we are indexing by
        if (!IndexName.str().empty())
        {
            IndexName << GIO_INDEX_SEP;
        }
        IndexName << pInfo->aConstraint[i].iColumn << indexType << opcode;

        // mark that we are using it to SQLite
        // (1 + the argv position that xFilter will get this value)
        pInfo->aConstraintUsage[i].argvIndex = 1 + IndexIndex++;

        // omit means to have SQLite ignore double-checking your work
        if (pInfo->aConstraint[i].iColumn == tab->RankColumnIndex)
        {
            pInfo->aConstraintUsage[i].omit = 1;
        }
        // TODO possibly add in a checks on our side to be able to omit
        // checking on the SQLite side - though the way that the
        // indices currently work, that would be difficult.
        else
        {
            pInfo->aConstraintUsage[i].omit = 0;
        }
    }

    // pass the index information out
    if (!IndexName.str().empty())
    {
        // we only have 1 logical index, everything is in the parameterization
        pInfo->idxNum = 1;
        pInfo->idxStr = sqlite3_mprintf("%s", IndexName.str().c_str());
        if (!pInfo->idxStr)
        {
            return SQLITE_NOMEM;
        }
        pInfo->needToFreeIdxStr = 1;

        // This is a crude estimate, without the actual range information,
        // this is the best that we can do.
        uint64_t TotalNumElems = tab->GIO.readTotalNumElems();
        if (TotalNumElems == (uint64_t) - 1)
        {
            TotalNumElems = ((double)tab->GIO.readNumElems(0)) *
                            tab->GIO.readNRanks();
        }
        // divide total number of rows by the number of used indices + 1
        pInfo->estimatedCost = (double)TotalNumElems / (IndexIndex + 1);
    }
    // no index
    else
    {
        uint64_t TotalNumElems = tab->GIO.readTotalNumElems();
        if (TotalNumElems == (uint64_t) - 1)
        {
            TotalNumElems = ((double)tab->GIO.readNumElems(0)) *
                            tab->GIO.readNRanks();
        }
        pInfo->estimatedCost = (double)TotalNumElems;
    }

    // if it is asking for an order by asc only on _rank or rowid
    if (pInfo->nOrderBy == 1)
    {
        if ((pInfo->aOrderBy[0].iColumn == tab->RankColumnIndex &&
                !pInfo->aOrderBy[0].desc) ||
                (pInfo->aOrderBy[0].iColumn == -1 &&
                 !pInfo->aOrderBy[0].desc))
        {
            pInfo->orderByConsumed = 1;
        }
    }
    else if (pInfo->nOrderBy == 2)
    {
        if ((pInfo->aOrderBy[0].iColumn == tab->RankColumnIndex &&
                !pInfo->aOrderBy[0].desc &&
                pInfo->aOrderBy[1].iColumn == -1 &&
                !pInfo->aOrderBy[1].desc) ||
                (pInfo->aOrderBy[1].iColumn == tab->RankColumnIndex &&
                 !pInfo->aOrderBy[1].desc &&
                 pInfo->aOrderBy[0].iColumn == -1 &&
                 !pInfo->aOrderBy[0].desc))
        {
            pInfo->orderByConsumed = 1;
        }
    }

    return SQLITE_OK;
}

static
int gio_open(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor)
{
    gio_cursor *cur = new gio_cursor(((gio_vtab*)pVTab)->FileName, (gio_vtab*)pVTab);

    if (!cur)
    {
        return SQLITE_ERROR;
    }

    cur->cursor.pVtab = pVTab;
    *ppCursor = &cur->cursor;

    return SQLITE_OK;
}

static
int gio_close(sqlite3_vtab_cursor* pCursor)
{
    gio_cursor *cur = (gio_cursor *) pCursor;

    delete cur;
    return SQLITE_OK;
}

static
int gio_filter(sqlite3_vtab_cursor* pCursor, int idxNum, const char *idxStr,
               int argc, sqlite3_value **argv)
{
    gio_cursor *cur = (gio_cursor *) pCursor;

    // initialize our cursor
    cur->RankMask.assign(cur->GIO.readNRanks(), true);
    cur->PercentageStart = 0.0;
    cur->PercentageEnd = 1.0;

    if (cur->HasOctree)
    {
        cur->OctreeMask.resize(cur->OctreeNodes->size());
        for (int i = 0; i < cur->GIO.readNRanks(); i++)
        {
            cur->OctreeMask[i].assign((*(cur->OctreeNodes))[i].size(), true);
        }
    }

    // we only have one logical index and encode parameterizations
    if (idxNum == 1)
    {
        stringstream ss(idxStr);
        string IndexSpec;
        for (int arg = 0; getline(ss, IndexSpec, GIO_INDEX_SEP); ++arg)
        {
            if (arg >= argc)
            {
                return SQLITE_ERROR;
            }

            // extract the index information
            stringstream tt(IndexSpec);
            int Col; char Type, Op;
            tt >> Col >> Type >> Op;

            // set indices for the rank mask
            if (Type == GIO_RANK_INDEX)
            {
                int Rank = sqlite3_value_int(argv[arg]);
                // set the rank mask
                for (int i = 0; i < (int)cur->RankMask.size(); ++i)
                {
                    switch (Op)
                    {
                    case GIO_EQUAL_CONSTRAINT:
                        cur->RankMask[i] = cur->RankMask[i] && (i == Rank);
                        break;
                    case GIO_GREAT_CONSTRAINT:
                        cur->RankMask[i] = cur->RankMask[i] && (i > Rank);
                        break;
                    case GIO_GRTEQ_CONSTRAINT:
                        cur->RankMask[i] = cur->RankMask[i] && (i >= Rank);
                        break;
                    case GIO_LESST_CONSTRAINT:
                        cur->RankMask[i] = cur->RankMask[i] && (i < Rank);
                        break;
                    case GIO_LESEQ_CONSTRAINT:
                        cur->RankMask[i] = cur->RankMask[i] && (i <= Rank);
                        break;
                    }
                }
            }
            else if (Type == GIO_PERCENT_INDEX)
            {
                // force use the octree index if we have it
                // this is to be consistent in our use of _percent
                // such that _percent is always the same,
                // even if we aren't using the index

                double fence = sqlite3_value_double(argv[arg]);

                switch (Op)
                {
                case GIO_EQUAL_CONSTRAINT:
                    cur->PercentageStart = fence;
                    cur->PercentageEnd = fence;
                    break;
                case GIO_GREAT_CONSTRAINT:
                    cur->PercentageStart = fence;
                    break;
                case GIO_GRTEQ_CONSTRAINT:
                    cur->PercentageStart = fence;
                    break;
                case GIO_LESST_CONSTRAINT:
                    cur->PercentageEnd = fence;
                    break;
                case GIO_LESEQ_CONSTRAINT:
                    cur->PercentageEnd = fence;
                    break;
                }
            }
            // we have an octree index
            else
            {
                double fence = sqlite3_value_double(argv[arg]);

                // set the octree mask
                for (int i = 0; i < cur->OctreeMask.size(); i++)
                {
                    for (int j = 0; j < cur->OctreeMask[i].size(); j++)
                    {
                        switch (Op)
                        {
                        case GIO_EQUAL_CONSTRAINT:
                            switch (Type)
                            {
                            case GIO_X_INDEX:
                                cur->OctreeMask[i][j] = cur->OctreeMask[i][j] &&
                                                        ((*(cur->OctreeNodes))[i][j].xMin <= fence &&
                                                         fence <= (*(cur->OctreeNodes))[i][j].xMax);
                                break;
                            case GIO_Y_INDEX:
                                cur->OctreeMask[i][j] = cur->OctreeMask[i][j] &&
                                                        ((*(cur->OctreeNodes))[i][j].yMin <= fence &&
                                                         fence <= (*(cur->OctreeNodes))[i][j].yMax);
                                break;
                            case GIO_Z_INDEX:
                                cur->OctreeMask[i][j] = cur->OctreeMask[i][j] &&
                                                        ((*(cur->OctreeNodes))[i][j].zMin <= fence &&
                                                         fence <= (*(cur->OctreeNodes))[i][j].zMax);
                                break;
                            }
                            break;
                        // I don't trust floating point, so it is >= for both
                        // SQLite will filter them out since we didn't set omit
                        case GIO_GREAT_CONSTRAINT:
                        case GIO_GRTEQ_CONSTRAINT:
                            switch (Type)
                            {
                            case GIO_X_INDEX:
                                cur->OctreeMask[i][j] = cur->OctreeMask[i][j] &&
                                                        (*(cur->OctreeNodes))[i][j].xMax >= fence;
                                break;
                            case GIO_Y_INDEX:
                                cur->OctreeMask[i][j] = cur->OctreeMask[i][j] &&
                                                        (*(cur->OctreeNodes))[i][j].yMax >= fence;
                                break;
                            case GIO_Z_INDEX:
                                cur->OctreeMask[i][j] = cur->OctreeMask[i][j] &&
                                                        (*(cur->OctreeNodes))[i][j].zMax >= fence;
                                break;
                            }
                            break;
                        // I don't trust floating point, so it is <= for both
                        // SQLite will filter them out since we didn't set omit
                        case GIO_LESST_CONSTRAINT:
                        case GIO_LESEQ_CONSTRAINT:
                            switch (Type)
                            {
                            case GIO_X_INDEX:
                                cur->OctreeMask[i][j] = cur->OctreeMask[i][j] &&
                                                        (*(cur->OctreeNodes))[i][j].xMin <= fence;
                                break;
                            case GIO_Y_INDEX:
                                cur->OctreeMask[i][j] = cur->OctreeMask[i][j] &&
                                                        (*(cur->OctreeNodes))[i][j].yMin <= fence;
                                break;
                            case GIO_Z_INDEX:
                                cur->OctreeMask[i][j] = cur->OctreeMask[i][j] &&
                                                        (*(cur->OctreeNodes))[i][j].zMin <= fence;
                                break;
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    // skip to the end if the percentages are out of range
    if (cur->PercentageStart > cur->PercentageEnd ||
            cur->PercentageEnd < 0.0 ||
            cur->PercentageStart > 1.0)
    {
        cur->Rank = cur->GIO.readNRanks();
        // return early
        goto exit1;
    }

    // initial row id
    cur->RowId = 1;
    cur->Index = 0;

    // we aren't using the octree index
    if (!cur->HasOctree)
    {
        // set the initial cursor based on our constraints
        for (cur->Rank = 0;
                cur->Rank < cur->GIO.readNRanks();
                cur->Rank++)
        {
            try
            {
                // if it's in the mask, read it
                if (cur->RankMask[cur->Rank] && cur->GIO.readNumElems(cur->Rank) > 0)
                {
                    // we use floor and ceil to make sure we don't miss any
                    // SQLite will filter out ones that don't match _percent
                    cur->Offset = (size_t)(floor((cur->GIO.readNumElems(cur->Rank) - 1) *
                                                 cur->PercentageStart));
                    size_t end = (size_t)(ceil(cur->GIO.readNumElems(cur->Rank) *
                                               cur->PercentageEnd));

                    cur->RowId += cur->Offset;
                    cur->Count = end - cur->Offset;
                    cur->Remain = cur->GIO.readNumElems(cur->Rank) - end;

                    cur->GIO.readDataSection(cur->Offset, cur->Count, cur->Rank, false);
                    break;
                }
                // else keep skipping
                else
                {
                    cur->RowId += cur->GIO.readNumElems(cur->Rank);
                }
            }
            catch (exception &e)
            {
                cerr << e.what() << endl;
                return SQLITE_IOERR;
            }
        }
    }
    // we are using the octree
    else
    {
        // set the initial cursor based on our constraints
        for (cur->Rank = 0;
                cur->Rank < cur->GIO.readNRanks();
                cur->Rank++)
        {
            try
            {
                // if it's in the rank mask, try reading it
                if (cur->RankMask[cur->Rank])
                {
                    for (cur->Leaf = 0;
                            cur->Leaf < cur->OctreeMask[cur->Rank].size();
                            cur->Leaf++)
                    {
                        // see if it's in the octree node
                        if (cur->OctreeMask[cur->Rank][cur->Leaf] &&
                                (*(cur->OctreeNodes))[cur->Rank][cur->Leaf].rows > 0)
                        {
                            // we use floor and ceil to make sure we don't miss any
                            // SQLite will filter out ones that don't match _percent
                            cur->Offset = (size_t)(floor((
                                                             (*(cur->OctreeNodes))[cur->Rank][cur->Leaf].rows - 1) *
                                                         cur->PercentageStart));
                            size_t end = (size_t)(ceil(
                                                      (*(cur->OctreeNodes))[cur->Rank][cur->Leaf].rows *
                                                      cur->PercentageEnd));

                            cur->RowId += cur->Offset;
                            cur->Count = end - cur->Offset;
                            cur->Remain = (*(cur->OctreeNodes))[cur->Rank][cur->Leaf].rows
                                          - end;

                            cur->GIO.readDataSection(
                                (*(cur->OctreeNodes))[cur->Rank][cur->Leaf].index +
                                cur->Offset,
                                cur->Count, cur->Rank, false);
                            // break out of doubly nested for loop
                            // (instead of having a flag -> break)
                            goto exit1;
                        }
                        // else keep skipping nodes
                        else
                        {
                            cur->RowId += (*(cur->OctreeNodes))[cur->Rank][cur->Leaf].rows;
                        }
                    }
                }
                // else keep skipping ranks
                else
                {
                    cur->RowId += cur->GIO.readNumElems(cur->Rank);
                }
            }
            catch (exception &e)
            {
                cerr << e.what() << endl;
                return SQLITE_IOERR;
            }
        }
    }
    exit1: // doubly nested for loop escape
    return SQLITE_OK;
}

static
int gio_next(sqlite3_vtab_cursor* pCursor)
{
    gio_cursor *cur = (gio_cursor *) pCursor;

    // always increment the row
    ++cur->RowId;
    ++cur->Index;


    // we aren't using the octree index
    if (!cur->HasOctree)
    {
        // unable to read the next thing
        // we reached the end of the rank, find the next rank
        if (cur->Index >= cur->Count)
        {
            // if there are left overs, increment
            cur->RowId += cur->Remain;
            cur->Index = 0;

            // goto the next rank
            cur->Rank++;
            for (;
                    cur->Rank < cur->GIO.readNRanks();
                    cur->Rank++)
            {
                try
                {
                    // if it's in rank, stop
                    if (cur->RankMask[cur->Rank] && cur->GIO.readNumElems(cur->Rank) > 0)
                    {
                        // we use floor and ceil to make sure we don't miss any
                        // SQLite will filter out ones that don't match _percent
                        cur->Offset = (size_t)(floor((cur->GIO.readNumElems(cur->Rank) - 1) *
                                                     cur->PercentageStart));
                        size_t end = (size_t)(ceil(cur->GIO.readNumElems(cur->Rank) *
                                                   cur->PercentageEnd));

                        cur->RowId += cur->Offset;
                        cur->Count = end - cur->Offset;
                        cur->Remain = cur->GIO.readNumElems(cur->Rank) - end;

                        cur->GIO.readDataSection(cur->Offset, cur->Count, cur->Rank, false);
                        break;
                    }
                    // else keep skipping
                    else
                    {
                        cur->RowId += cur->GIO.readNumElems(cur->Rank);
                    }
                }
                catch (exception &e)
                {
                    cerr << e.what() << endl;
                    return SQLITE_IOERR;
                }
            }
        }
    }
    // we are using the octree
    else
    {
        // unable to read the next thing
        // we reached the end of the node, find the next node
        if (cur->Index >= cur->Count)
        {
            // if there are left overs, increment
            cur->RowId += cur->Remain;
            cur->Index = 0;

            // goto the next leaf
            cur->Leaf++;
            for (;
                    cur->Rank < cur->GIO.readNRanks();
                    cur->Rank++)
            {
                try
                {
                    // if it's in the rank mask, try reading it
                    if (cur->RankMask[cur->Rank])
                    {
                        for (;
                                cur->Leaf < cur->OctreeMask[cur->Rank].size();
                                cur->Leaf++)
                        {
                            // see if it's in the octree node
                            if (cur->OctreeMask[cur->Rank][cur->Leaf] && (*(cur->OctreeNodes))[cur->Rank][cur->Leaf].rows > 0)
                            {
                                // we use floor and ceil to make sure we don't miss any
                                // SQLite will filter out ones that don't match _percent
                                cur->Offset = (size_t)(floor((
                                                                 (*(cur->OctreeNodes))[cur->Rank][cur->Leaf].rows - 1) *
                                                             cur->PercentageStart));
                                size_t end = (size_t)(ceil(
                                                          (*(cur->OctreeNodes))[cur->Rank][cur->Leaf].rows *
                                                          cur->PercentageEnd));

                                cur->RowId += cur->Offset;
                                cur->Count = end - cur->Offset;
                                cur->Remain = (*(cur->OctreeNodes))[cur->Rank][cur->Leaf].rows
                                              - end;

                                cur->GIO.readDataSection(
                                    (*(cur->OctreeNodes))[cur->Rank][cur->Leaf].index +
                                    cur->Offset,
                                    cur->Count, cur->Rank, false);
                                // break out of doubly nested for loop
                                // (instead of having a flag -> break)
                                goto exit2;
                            }
                            // else keep skipping nodes
                            else
                            {
                                cur->RowId += (*(cur->OctreeNodes))[cur->Rank][cur->Leaf].rows;
                            }
                        }

                        // we reached the end of the leaves, wrap around
                        cur->Leaf = 0;
                    }
                    // go to the next rank
                    else
                    {
                        cur->RowId += cur->GIO.readNumElems(cur->Rank);
                        cur->Rank++;
                    }
                }
                catch (exception &e)
                {
                    cerr << e.what() << endl;
                    return SQLITE_IOERR;
                }
            }
        }
    }
    exit2: // doubly nested for loop escape
    return SQLITE_OK;
}

static
int gio_eof(sqlite3_vtab_cursor* pCursor)
{
    gio_cursor *cur = (gio_cursor *) pCursor;

    if (cur->Rank >= cur->GIO.readNRanks())
    {
        return 1;
    }

    return 0;
}

static
int gio_column(sqlite3_vtab_cursor* pCursor, sqlite3_context* cxt, int n)
{
    gio_cursor* cur = (gio_cursor*) pCursor;

    // _rank column
    if (n == cur->RankColumnIndex)
    {
        sqlite3_result_int(cxt, cur->Rank);
    }
    // _percent column
    else if (n == cur->PercentColumnIndex)
    {
        if (!cur->HasOctree)
        {
            sqlite3_result_double(cxt,
                                  ((double)cur->Index + cur->Offset) /
                                  cur->GIO.readNumElems(cur->Rank));
        }
        else
        {
            sqlite3_result_double(cxt,
                                  ((double)cur->Index + cur->Offset) /
                                  (*(cur->OctreeNodes))[cur->Rank][cur->Leaf].rows);
        }
    }
    // all other columns (actual GIO data)
    else
    {
        if ((cur->Printers)[n])
        {
            (cur->Printers)[n]->print(cxt, cur->Index);
        }
        else
        {
            sqlite3_result_null(cxt);
        }
    }

    return SQLITE_OK;
}

static
int gio_rowid(sqlite3_vtab_cursor* pCursor, sqlite3_int64* pRowid)
{
    gio_cursor* cur = (gio_cursor*) pCursor;
    *pRowid = (sqlite3_int64) cur->RowId;

    return SQLITE_OK;
}

extern "C"
int sq3_register_virtual_gio(sqlite3* db)
{
    static const struct sqlite3_module gio_module =
    {
        2,                         /* iVersion */
        gio_create,
        gio_connect,
        gio_best_index,
        gio_disconnect,
        gio_destroy,
        gio_open,
        gio_close,
        gio_filter,
        gio_next,
        gio_eof,
        gio_column,
        gio_rowid,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0
    };

    return sqlite3_create_module( db, "GenericIO", &gio_module, NULL );
}

extern "C"
int sqlite3_extension_init(sqlite3 *db, char **,
                           const sqlite3_api_routines *pApi)
{
    SQLITE_EXTENSION_INIT2(pApi);
    sq3_register_virtual_gio(db);
    return SQLITE_OK;
}

/*
Example script:

sqlite3
.headers on
.mode csv

      /projects/groups/vizproject/ExaSky/VizAly/genericio/frontend/GenericIOSQLite.so
.load /home/pascal/projects/VizAly/genericio/frontend/GenericIOSQLite.so
CREATE VIRTUAL TABLE foo USING GenericIO("/home/pascal/projects/VizAly/DataGenerator/outputFileOct");
PRAGMA table_info(foo) ;

SELECT count(*) FROM foo ;
SELECT count(*) FROM foo where _rank = 0 ;

/projects/groups/vizproject/ExaSky/VizAly/genericio/frontend/GenericIOSQLite.so
*/