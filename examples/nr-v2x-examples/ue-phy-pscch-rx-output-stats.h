/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#ifndef UE_PHY_PSCCH_RX_OUTPUT_STATS_H
#define UE_PHY_PSCCH_RX_OUTPUT_STATS_H

#include <vector>

#include <ns3/sqlite-output.h>
#include <ns3/nr-sl-phy-mac-common.h>

namespace ns3 {

/**
 * \brief Class to listen and store the RxPscchTraceUe trace of NrSpectrumPhy
 *        into a database.
 *
 * \see SetDb
 * \see Save
 */
class UePhyPscchRxOutputStats
{
public:
  /**
   * \brief UePhyPscchRxOutputStats constructor
   */
  UePhyPscchRxOutputStats ();

  /**
   * \brief Install the output database for PSCCH RX trace from NrSpectrumPhy.
   *
   * \param db database pointer
   * \param tableName name of the table where the values will be stored
   *
   * The db pointer must be valid through all the lifespan of the class. The
   * method creates, if not exists, a table for storing the values. The table
   * will contain the following columns:
   *
   * - "timeMs DOUBLE NOT NULL, "
   * - "cellId INTEGER NOT NULL,"
   * - "rnti INTEGER NOT NULL,"
   * - "bwpId INTEGER NOT NULL,"
   * - "frame INTEGER NOT NULL,"
   * - "subFrame INTEGER NOT NULL,"
   * - "slot INTEGER NOT NULL,"
   * - "txRnti INTEGER NOT NULL,"
   * - "dstL2Id INTEGER NOT NULL,"
   * - "pscchRbStart INTEGER NOT NULL,"
   * - "pscchRbLen INTEGER NOT NULL,"   *
   * - "pscchMcs INTEGER NOT NULL,"
   * - "avrgSinr INTEGER NOT NULL,"
   * - "minSinr INTEGER NOT NULL,"
   * - "tbler INTEGER NOT NULL,"
   * - "corrupt INTEGER NOT NULL,"
   * - "psschStartSbCh INTEGER NOT NULL,"
   * - "psschLenSbCh INTEGER NOT NULL,"
   * - "maxNumPerReserve INTEGER NOT NULL,"
   * - "rsvpMs INTEGER NOT NULL,"
   * - "SEED INTEGER NOT NULL,"
   * - "RUN INTEGER NOT NULL"
   *
   * Please note that this method, if the db already contains a table with
   * the same name, also clean existing values that has the same
   * Seed/Run pair.
   */
  void SetDb (SQLiteOutput *db, const std::string & tableName);

  /**
   * \brief Store the PSCCH stats parameters into a local vector, which
   *        acts as a cache.
   *
   * \param pscchStatsParams The PSCCH stats parameters
   */
  void Save (const SlRxCtrlPacketTraceParams pscchStatsParams);

  /**
   * \brief Force the cache write to disk, emptying the cache itself.
   */
  void EmptyCache ();

private:
  /**
   * \brief Delete the table if it already exists with same seed and run number
   * \param p The pointer to the DB
   * \param seed The seed index
   * \param run The run index
   * \param table The name of the table
   */
  static void DeleteWhere (SQLiteOutput *p, uint32_t seed, uint32_t run, const std::string &table);
  /**
   * \brief Write the data stored in our local cache into the DB
   */
  void WriteCache ();

  SQLiteOutput *m_db {nullptr}; //!< DB pointer
  std::string m_tableName {"IvalidTableName"}; //!< table name
  std::vector<SlRxCtrlPacketTraceParams> m_pscchCache;   //!< Result cache
};

} // namespace ns3

#endif // UE_PHY_PSCCH_RX_OUTPUT_STATS_H
