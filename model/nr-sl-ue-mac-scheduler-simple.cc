﻿/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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

#include "nr-sl-ue-mac-scheduler-simple.h"

#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrSlUeMacSchedulerSimple");
NS_OBJECT_ENSURE_REGISTERED (NrSlUeMacSchedulerSimple);

NrSlUeMacSchedulerSimple::NrSlUeMacSchedulerSimple ()
{
}

TypeId
NrSlUeMacSchedulerSimple::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrSlUeMacSchedulerSimple")
    .SetParent<NrSlUeMacSchedulerNs3> ()
    .AddConstructor<NrSlUeMacSchedulerSimple> ()
    .SetGroupName ("nr")
  ;
  return tid;
}

bool
NrSlUeMacSchedulerSimple::DoNrSlAllocation (const std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>& txOpps,
                                            const std::shared_ptr<NrSlUeMacSchedulerDstInfo> &dstInfo,
                                            NrSlSlotAlloc &slotAlloc)
{
  NS_LOG_FUNCTION (this);
  bool allocated = false;

  const auto & lcgMap = dstInfo->GetNrSlLCG (); //Map of unique_ptr should not copy

  NS_ASSERT_MSG (lcgMap.size () == 1, "NrSlUeMacSchedulerSimple can handle only one LCG");

  std::vector<uint8_t> lcVector = lcgMap.begin ()->second->GetLCId ();
  NS_ASSERT_MSG (lcVector.size () == 1, "NrSlUeMacSchedulerSimple can handle only one LC");

  uint32_t bufferSize = lcgMap.begin ()->second->GetTotalSizeOfLC (lcVector.at (0));

  if (bufferSize == 0)
    {
      return allocated;
    }

  std::set <uint16_t> randTxOpps = RandomlySelectSlots (txOpps);

  std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>::const_iterator txOppsIt = txOpps.begin ();
  std::advance (txOppsIt, *(randTxOpps.begin ()));

  NS_ASSERT_MSG (randTxOpps.size () == txOppsIt->slMaxNumPerReserve, "Number of randomly chosen slots should be equal to slMaxNumPerReserve");

  uint32_t tbs = 0;
  uint8_t assignedSbCh = 0;
  uint16_t availableSymbols = txOppsIt->slPsschSymLength;
  NS_LOG_DEBUG ("Total available symbols for PSSCH = " << availableSymbols);
  do
    {
      assignedSbCh++;
      tbs = GetNrSlAmc ()->CalculateTbSize (dstInfo->GetDstMcs (), txOppsIt->slSubchannelSize * assignedSbCh * availableSymbols);
    }
  while (tbs < bufferSize + 5 /*(5 bytes overhead of SCI format 2A)*/ && (GetTotalSubCh () - assignedSbCh) > 0);

  //Now, before allocating bytes to LCs we subtract 5 bytes for SCI format 2A
  //since we already took it into account while computing the TB size.
  tbs = tbs - 5 /*(5 bytes overhead of SCI stage 2)*/;

  allocated = true;
  slotAlloc.sfn = txOppsIt->sfn;
  slotAlloc.dstL2Id = dstInfo->GetDstL2Id ();
  slotAlloc.ndi = 1;
  slotAlloc.rv = 0;
  slotAlloc.priority = lcgMap.begin ()->second->GetLcPriority (lcVector.at (0));
  SlRlcPduInfo slRlcPduInfo (lcVector.at (0), tbs);
  slotAlloc.slRlcPduInfo.push_back (slRlcPduInfo);
  slotAlloc.mcs = dstInfo->GetDstMcs ();
  //PSCCH
  slotAlloc.numSlPscchRbs = txOppsIt->numSlPscchRbs;
  slotAlloc.slPscchSymStart = txOppsIt->slPscchSymStart;
  slotAlloc.slPscchSymLength = txOppsIt->slPscchSymLength;
  //PSSCH
  slotAlloc.slPsschSymStart = txOppsIt->slPsschSymStart;
  slotAlloc.slPsschSymLength = availableSymbols;
  slotAlloc.slPsschSubChStart = 0;
  slotAlloc.slPsschSubChLength = assignedSbCh;
  slotAlloc.maxNumPerReserve = txOppsIt->slMaxNumPerReserve;

  if (randTxOpps.size () > 1)
    {
      txOppsIt = txOpps.begin ();
      NS_LOG_DEBUG ("Advancing to " << *(std::next (randTxOpps.begin (), 1)) << " index for ReTxGap1");
      std::advance (txOppsIt, *(std::next (randTxOpps.begin (), 1)));
      uint64_t gapReTx1 = txOppsIt->sfn.Normalize () - slotAlloc.sfn.Normalize ();
      NS_LOG_DEBUG ("Gap between first Tx and first ReTx is absolute slots = " << gapReTx1);
      NS_ABORT_IF (gapReTx1 > UINT8_MAX);
      slotAlloc.gapReTx1 = static_cast <uint8_t> (gapReTx1);
    }

  if (randTxOpps.size () > 2)
    {
      txOppsIt = txOpps.begin ();
      NS_LOG_DEBUG ("Advancing to " << *(std::next (randTxOpps.begin (), 2)) << " index for ReTxGap2");
      std::advance (txOppsIt, *(std::next (randTxOpps.begin (), 2)));
      uint64_t gapReTx2 = txOppsIt->sfn.Normalize () - slotAlloc.sfn.Normalize ();
      NS_LOG_DEBUG ("Gap between first Tx and second ReTx is absolute slots = " << gapReTx2);
      NS_ABORT_IF (gapReTx2 > UINT8_MAX);
      slotAlloc.gapReTx2 = static_cast <uint8_t> (gapReTx2);
    }

  lcgMap.begin ()->second->AssignedData (lcVector.at (0), tbs);
  return allocated;
}


std::set <uint16_t>
NrSlUeMacSchedulerSimple::RandomlySelectSlots (const std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>& txOpps)
{
  NS_LOG_FUNCTION (this);

  std::set <uint16_t> randIndex;

  std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>::const_iterator txOppsIt = txOpps.begin ();

  NS_ASSERT_MSG (txOpps.size () >= txOppsIt->slMaxNumPerReserve,
                 "not enough txOpps to perform " << txOppsIt->slMaxNumPerReserve << " transmissions");

  NS_ASSERT_MSG (txOppsIt->slMaxNumPerReserve >= 1 && txOppsIt->slMaxNumPerReserve < 4,
                 "slMaxNumPerReserve should be greater than 1 and less than 4");

  uint16_t totalReTx = txOppsIt->slMaxNumPerReserve - 1;
  uint16_t txOppSize = static_cast <uint16_t> (txOpps.size ());
  uint16_t reTxWindSize = static_cast <uint16_t> (GetNrSlReTxWindow ());

  uint16_t firstTxSlot = m_uniformVariable->GetInteger (1, (txOppSize - totalReTx));

  uint32_t firstTxIndex = firstTxSlot - 1; //adjust since list index start from 0

  randIndex.insert (firstTxIndex);

  if (txOppsIt->slMaxNumPerReserve == 1)
    {
      return randIndex;
    }

  uint16_t remainingTxSlots = txOppSize - firstTxSlot;

  uint16_t finalRetxWind = std::min (reTxWindSize, remainingTxSlots);

  uint16_t lastSlotForRetxOne = (finalRetxWind - totalReTx) + 1 + firstTxSlot;

  uint16_t reTxOneSlot = m_uniformVariable->GetInteger ((firstTxSlot + 1), lastSlotForRetxOne);

  uint16_t reTxOneIndex = reTxOneSlot - 1; //adjust since list index start from 0

  randIndex.insert (reTxOneIndex);

  if (txOppsIt->slMaxNumPerReserve == 2)
    {
      return randIndex;
    }

  uint16_t lastSlotForRetxTwo = firstTxSlot + finalRetxWind;

  uint16_t reTxTwoSlot = m_uniformVariable->GetInteger ((reTxOneSlot + 1), lastSlotForRetxTwo);

  uint16_t reTxTwoIndex = reTxTwoSlot - 1; //adjust since list index start from 0

  randIndex.insert (reTxTwoIndex);
  return randIndex;
}



} //namespace ns3