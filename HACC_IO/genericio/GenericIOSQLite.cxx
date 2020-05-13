/*
 *                    Copyright (C) 2015, UChicago Argonne, LLC
 *                               All Rights Reserved
 * 
 *                               Generic IO (ANL-15-066)
 *                     Hal Finkel, Argonne National Laboratory
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
SQLITE_EXTENSION_INIT1

using namespace std;
using namespace gio;

class PrinterBase {
public:
  virtual ~PrinterBase() {}
  virtual void print(sqlite3_context *cxt, size_t i) = 0;
};

template <class T>
class Printer : public PrinterBase {
public:
  Printer(GenericIO &G, size_t MNE, const string &N)
    : Data(MNE + G.requestedExtraSpace()/sizeof(T)) {
    G.addVariable(N, Data, true);
  }

  virtual void print(sqlite3_context *cxt, size_t i) {
    T d = Data[i];
    if (!numeric_limits<T>::is_integer) {
      sqlite3_result_double(cxt, double(d));
    } else {
      if (sizeof(T) > 4) {
        sqlite3_result_int64(cxt, (sqlite3_int64) d);
      } else {
        sqlite3_result_int(cxt, int(d));
      }
    }
  }

protected:
  vector<T> Data;
};

template <typename T>
PrinterBase *addPrinter(GenericIO::VariableInfo &V,
                GenericIO &GIO, size_t MNE) {
  if (sizeof(T) != V.Size)
    return 0;

  if (V.IsFloat == numeric_limits<T>::is_integer)
    return 0;
  if (V.IsSigned != numeric_limits<T>::is_signed)
    return 0;

  return new Printer<T>(GIO, MNE, V.Name);
}

// See: Michael Owens. Query Anything with SQLite.
// Dr. Dobb's Journal. Nov. 2007.
// http://www.drdobbs.com/database/query-anything-with-sqlite/202802959
// See: http://www.sqlite.org/c3ref/load_extension.html
// See: http://www.sqlite.org/vtab.html

struct gio_vtab {
  gio_vtab(const string &FN) : GIO(FN), MaxNumElems(0) {
    memset(&vtab, 0, sizeof(sqlite3_vtab));

    GIO.openAndReadHeader(false);

    int NR = GIO.readNRanks();
    for (int i = 0; i < NR; ++i) {
      size_t NElem = GIO.readNumElems(i);
      MaxNumElems = max(MaxNumElems, NElem);
    }
  }

  sqlite3_vtab vtab;
  GenericIO    GIO;
  size_t       MaxNumElems;
};

struct gio_cursor {
  gio_cursor(const GenericIO &G, size_t MaxNElem) : GIO(G) {
    memset(&cursor, 0, sizeof(sqlite3_vtab_cursor));

    vector<GenericIO::VariableInfo> VI;
    GIO.getVariableInfo(VI);

    Vars.resize(VI.size());
    for (size_t i = 0; i < VI.size(); ++i) {
      PrinterBase *P = 0;

#define ADD_PRINTER(T) \
      if (!P) P = addPrinter<T>(VI[i], GIO, MaxNElem)
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

      if (P) Printers.push_back(P);
    }
  }

  sqlite3_vtab_cursor    cursor;
  int                    Rank;
  size_t                 Pos;
  uint64_t               AbsPos;
  GenericIO              GIO;
  vector< vector<char> > Vars;
  vector<PrinterBase *>  Printers;
  vector<bool>           RankMask;
};

static
int gio_connect(sqlite3* db, void *, int argc, const char *const *argv,
               sqlite3_vtab **ppVTab, char **pzErr) {
  if (argc < 4) {
    *pzErr = sqlite3_mprintf("%s", "No input file specified!");
    return SQLITE_ERROR;
  }

  *pzErr = 0;
  try {
    string FileName(argv[3]);

    gio_vtab *tab = new gio_vtab(FileName);
    if (!tab)
      return SQLITE_ERROR;

    vector<GenericIO::VariableInfo> VI;
    tab->GIO.getVariableInfo(VI);

    string Schema = "CREATE TABLE x(_rank INTEGER";
    if (VI.size())
      Schema += ", ";

    for (size_t i = 0; i < VI.size(); ++i) {
      if (VI[i].Size > 8)
        continue;

      Schema += VI[i].Name + (VI[i].IsFloat ? " REAL" : " INTEGER");
      if (i != VI.size() - 1)
        Schema += ", ";
    }

    Schema += ")";
    if (sqlite3_declare_vtab(db, Schema.c_str()) != SQLITE_OK) {
      *pzErr = sqlite3_mprintf("Could not declare schema: %s", Schema.c_str());
      delete tab;
      return SQLITE_ERROR;
    }

    *ppVTab = &tab->vtab;
  } catch(exception &e) {
     *pzErr = sqlite3_mprintf("%s", e.what());
     return SQLITE_ERROR;
  }

  return SQLITE_OK;
}

static
int gio_create(sqlite3* db, void *pAux, int argc, const char *const *argv,
               sqlite3_vtab **ppVTab, char **pzErr) {
  return gio_connect(db, pAux, argc, argv, ppVTab, pzErr);
}

static
int gio_disconnect(sqlite3_vtab *pVTab) {
  gio_vtab *tab = (gio_vtab *) pVTab;

  delete tab;
  return SQLITE_OK;
}

static
int gio_destroy(sqlite3_vtab *pVTab) {
  return gio_disconnect(pVTab);
}

static
int gio_best_index(sqlite3_vtab *pVTab, sqlite3_index_info *pInfo) {
  gio_vtab *tab = (gio_vtab *) pVTab;

  int IndexIndex = 0;
  stringstream IndexName;
  for (int i = 0; i < pInfo->nConstraint; ++i) {
    if (!pInfo->aConstraint[i].usable)
      continue;

    char opcode = 0;
    switch (pInfo->aConstraint[i].op) {
    case SQLITE_INDEX_CONSTRAINT_EQ:
      opcode = 'E';
      break;
    case SQLITE_INDEX_CONSTRAINT_GT:
      opcode = 'g';
      break;
    case SQLITE_INDEX_CONSTRAINT_LT:
      opcode = 'l';
      break;
    case SQLITE_INDEX_CONSTRAINT_GE:
      opcode = 'G';
      break;
    case SQLITE_INDEX_CONSTRAINT_LE:
      opcode = 'L';
      break;
    }

    if (!opcode)
      continue;

    // For now, we only support column 0 (_rank)
    // TODO: support for x, y, z, etc.
    if (pInfo->aConstraint[i].iColumn != 0)
      continue;

    if (!IndexName.str().empty())
      IndexName << ";";
    IndexName << pInfo->aConstraint[i].iColumn << ":" << opcode;

    pInfo->aConstraintUsage[i].argvIndex = 1 + IndexIndex++;
    pInfo->aConstraintUsage[i].omit = 1;
  }

  if (!IndexName.str().empty()) {
    pInfo->idxNum = 1;
    pInfo->idxStr = sqlite3_mprintf("%s", IndexName.str().c_str());
    if (!pInfo->idxStr)
      return SQLITE_NOMEM;

    pInfo->needToFreeIdxStr = 1;

    // This is a crude estimate, without the actual range information,
    // this is the best that we can do.
    if (tab->GIO.readNRanks() > 0)
      pInfo->estimatedCost = (double) tab->GIO.readNumElems(0);
  } else {
    uint64_t TotalNumElems = tab->GIO.readTotalNumElems();
    if (TotalNumElems == (uint64_t) -1)
      TotalNumElems = ((double) tab->GIO.readNumElems(0))*
                        tab->GIO.readNRanks();
    pInfo->estimatedCost = (double) TotalNumElems;
  }

  if (pInfo->nOrderBy == 1 && pInfo->aOrderBy[0].iColumn == 0 &&
      pInfo->aOrderBy[0].desc == 0 /* ASC */)
    pInfo->orderByConsumed = 1;

  return SQLITE_OK;
}

static
int gio_open(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor) {
  gio_vtab   *tab = (gio_vtab *) pVTab;
  gio_cursor *cur = new gio_cursor(tab->GIO, tab->MaxNumElems);
  if (!cur)
    return SQLITE_ERROR;

  cur->cursor.pVtab = pVTab;
  *ppCursor = &cur->cursor;

  return SQLITE_OK;
}

static
int gio_close(sqlite3_vtab_cursor* pCursor) {
  gio_cursor *cur = (gio_cursor *) pCursor;

  delete cur;
  return SQLITE_OK;
}

static
int gio_filter(sqlite3_vtab_cursor* pCursor, int idxNum, const char *idxStr,
               int argc, sqlite3_value **argv) {
  gio_cursor *cur = (gio_cursor *) pCursor;
  gio_vtab   *tab = (gio_vtab *) cur->cursor.pVtab;

  cur->RankMask.resize(cur->GIO.readNRanks(), true);
  if (idxNum == 1) {
    stringstream ss(idxStr);
    string IndexSpec;
    for (int arg = 0; getline(ss, IndexSpec, ';'); ++arg) {
      if (arg >= argc)
        return SQLITE_ERROR;

      stringstream tt(IndexSpec);
      int Col; char Colon, Op;
      tt >> Col >> Colon >> Op;
      if (Colon != ':')
        return SQLITE_ERROR;

      // TODO: Support for other columns.
      if (Col != 0)
        return SQLITE_ERROR;

      int Rank = sqlite3_value_int(argv[arg]);
      for (int i = 0; i < (int) cur->RankMask.size(); ++i) {
        bool Incl = false;
        if (Op == 'E')
          Incl = (i == Rank);
        else if (Op == 'g')
          Incl = (i > Rank);
        else if (Op == 'G')
          Incl = (i >= Rank);
        else if (Op == 'l')
          Incl = (i < Rank);
        else if (Op == 'L')
          Incl = (i <= Rank);

        cur->RankMask[i] = cur->RankMask[i] && Incl;
      }
    }
  }

  cur->Pos = 0;
  cur->AbsPos = 0;

  cur->Rank = -1;
  do {
    try {
      ++cur->Rank;
      if (cur->Rank == cur->GIO.readNRanks()) {
        break;
      }

      if (cur->RankMask[cur->Rank])
        cur->GIO.readData(cur->Rank, false);
      else
        cur->AbsPos += cur->GIO.readNumElems(cur->Rank);
    } catch (exception &e) {
        cerr << e.what() << endl;
        return SQLITE_IOERR;
    }
  } while (!cur->RankMask[cur->Rank] ||
           cur->GIO.readNumElems(cur->Rank) == 0);

  return SQLITE_OK;
}

static
int gio_next(sqlite3_vtab_cursor* pCursor) {
  gio_cursor *cur = (gio_cursor *) pCursor;

  if (cur->Pos == cur->GIO.readNumElems(cur->Rank) - 1) {
    do {
      try {
        ++cur->Rank;
        if (cur->Rank == cur->GIO.readNRanks()) {
          break;
        }

        if (cur->RankMask[cur->Rank])
          cur->GIO.readData(cur->Rank, false);
        else
          cur->AbsPos += cur->GIO.readNumElems(cur->Rank);
      } catch (exception &e) {
        cerr << e.what() << endl;
        return SQLITE_IOERR;
      }
    } while (!cur->RankMask[cur->Rank] ||
             cur->GIO.readNumElems(cur->Rank) == 0);

    cur->Pos = 0;
  } else {
    ++cur->Pos;
  }

  ++cur->AbsPos;

  return SQLITE_OK;
}

static
int gio_eof(sqlite3_vtab_cursor* pCursor) {
  gio_cursor *cur = (gio_cursor *) pCursor;

  if (cur->Rank == cur->GIO.readNRanks())
    return 1;

  if (cur->Rank == cur->GIO.readNRanks() - 1 &&
      cur->Pos >= (cur->GIO.readNumElems(cur->Rank) - 1))
    return 1;

  return 0;
}

static
int gio_column(sqlite3_vtab_cursor* pCursor, sqlite3_context* cxt, int n) {
  gio_cursor *cur = (gio_cursor *) pCursor;

  if (n == 0)
    sqlite3_result_int(cxt, cur->Rank);
  else if ((n-1) >= cur->Printers.size())
    sqlite3_result_null(cxt);
  else
    cur->Printers[n-1]->print(cxt, cur->Pos);

  return SQLITE_OK;
}

static
int gio_rowid(sqlite3_vtab_cursor* pCursor, sqlite3_int64 *pRowid) {
  gio_cursor *cur = (gio_cursor *) pCursor;

  *pRowid = (sqlite3_int64) cur->AbsPos;
  return SQLITE_OK;
}

extern "C"
int sq3_register_virtual_gio(sqlite3 * db) {
  static const struct sqlite3_module gio_module = {
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
                           const sqlite3_api_routines *pApi) {
  SQLITE_EXTENSION_INIT2(pApi);
  sq3_register_virtual_gio(db);
  return SQLITE_OK; 
}

