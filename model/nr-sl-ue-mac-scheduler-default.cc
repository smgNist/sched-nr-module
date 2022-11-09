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

#include "nr-sl-ue-mac-scheduler-default.h"

#include <ns3/log.h>
#include <ns3/boolean.h>
#include <ns3/uinteger.h>
#include <ns3/pointer.h>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrSlUeMacSchedulerDefault");
NS_OBJECT_ENSURE_REGISTERED (NrSlUeMacSchedulerDefault);

TypeId
NrSlUeMacSchedulerDefault::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrSlUeMacSchedulerDefault")
    .SetParent<NrSlUeMacScheduler> ()
    .AddConstructor<NrSlUeMacSchedulerDefault> ()
    .SetGroupName ("nr")
    .AddAttribute ("FixNrSlMcs",
                   "Fix MCS to value set in SetInitialNrSlMcs",
                   BooleanValue (false),
                   MakeBooleanAccessor (&NrSlUeMacSchedulerDefault::UseFixedNrSlMcs,
                                        &NrSlUeMacSchedulerDefault::IsNrSlMcsFixed),
                   MakeBooleanChecker ())
    .AddAttribute ("InitialNrSlMcs",
                   "The initial value of the MCS used for NR Sidelink",
                   UintegerValue (14),
                   MakeUintegerAccessor (&NrSlUeMacSchedulerDefault::SetInitialNrSlMcs,
                                         &NrSlUeMacSchedulerDefault::GetInitialNrSlMcs),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("NrSlAmc",
                   "The NR SL AMC of this scheduler",
                   PointerValue (),
                   MakePointerAccessor (&NrSlUeMacSchedulerDefault::m_nrSlAmc),
                   MakePointerChecker <NrAmc> ())
  ;
  return tid;
}

NrSlUeMacSchedulerDefault::NrSlUeMacSchedulerDefault ()
{
  m_uniformVariable = CreateObject<UniformRandomVariable> ();
}

NrSlUeMacSchedulerDefault::~NrSlUeMacSchedulerDefault ()
{
  //just to make sure
  m_dstMap.clear ();
}

void
NrSlUeMacSchedulerDefault::DoCschedUeNrSlLcConfigReq (const NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo& params)
{
  NS_LOG_FUNCTION (this << params.dstL2Id << +params.lcId);

  auto dstInfo = CreateDstInfo (params);
  const auto & lcgMap = dstInfo->GetNrSlLCG (); //Map of unique_ptr should not copy
  auto itLcg = lcgMap.find (params.lcGroup);
  auto itLcgEnd = lcgMap.end ();
  if (itLcg == itLcgEnd)
    {
      NS_LOG_DEBUG ("Created new NR SL LCG for destination " << dstInfo->GetDstL2Id () <<
                    " LCG ID =" << static_cast<uint32_t> (params.lcGroup));
      itLcg = dstInfo->Insert (CreateLCG (params.lcGroup));
    }

  itLcg->second->Insert (CreateLC (params));
  NS_LOG_INFO ("Added LC id " << +params.lcId << " in LCG " << +params.lcGroup);
  //send confirmation to UE MAC
  m_nrSlUeMacCschedSapUser->CschedUeNrSlLcConfigCnf (params.lcGroup, params.lcId);
}

std::shared_ptr<NrSlUeMacSchedulerDstInfo>
NrSlUeMacSchedulerDefault::CreateDstInfo (const NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo& params)
{
  std::shared_ptr<NrSlUeMacSchedulerDstInfo> dstInfo = nullptr;
  auto itDst = m_dstMap.find (params.dstL2Id);
  if (itDst == m_dstMap.end ())
    {
      NS_LOG_INFO ("Creating destination info. Destination L2 id " << params.dstL2Id);

      dstInfo = std::make_shared <NrSlUeMacSchedulerDstInfo> (params.dstL2Id);
      dstInfo->SetDstMcs (m_initialNrSlMcs);

      itDst = m_dstMap.insert (std::make_pair (params.dstL2Id, dstInfo)).first;
    }
  else
    {
      NS_LOG_LOGIC ("Doing nothing. You are seeing this because we are adding new LC " << +params.lcId << " for Dst " << params.dstL2Id);
      dstInfo = itDst->second;
    }

  return dstInfo;
}


NrSlLCGPtr
NrSlUeMacSchedulerDefault::CreateLCG (uint8_t lcGroup) const
{
  NS_LOG_FUNCTION (this);
  return std::unique_ptr<NrSlUeMacSchedulerLCG> (new NrSlUeMacSchedulerLCG (lcGroup));
}


NrSlLCPtr
NrSlUeMacSchedulerDefault::CreateLC (const NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo& params) const
{
  NS_LOG_FUNCTION (this);
  return std::unique_ptr<NrSlUeMacSchedulerLC> (new NrSlUeMacSchedulerLC (params));
}


void
NrSlUeMacSchedulerDefault::DoSchedUeNrSlRlcBufferReq (const struct NrSlUeMacSchedSapProvider::SchedUeNrSlReportBufferStatusParams& params)
{
  NS_LOG_FUNCTION (this << params.dstL2Id <<
                   static_cast<uint32_t> (params.lcid));

  GetSecond DstInfoOf;
  auto itDst = m_dstMap.find (params.dstL2Id);
  NS_ABORT_MSG_IF (itDst == m_dstMap.end (), "Destination " << params.dstL2Id << " info not found");

  for (const auto &lcg : DstInfoOf (*itDst)->GetNrSlLCG ())
    {
      if (lcg.second->Contains (params.lcid))
        {
          NS_LOG_INFO ("Updating NR SL LC Info: " << params <<
                       " in LCG: " << static_cast<uint32_t> (lcg.first));
          lcg.second->UpdateInfo (params);
          return;
        }
    }
  // Fail miserably because we didn't find any LC
  NS_FATAL_ERROR ("The LC does not exist. Can't update");
}

void
NrSlUeMacSchedulerDefault::DoSchedUeNrSlTriggerReq (uint32_t dstL2Id, const std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>& params)
{
  NS_LOG_FUNCTION (this << dstL2Id);

  const auto itDst = m_dstMap.find (dstL2Id);
  NS_ABORT_MSG_IF (itDst == m_dstMap.end (), "Destination " << dstL2Id << "info not found");

  std::set<NrSlSlotAlloc> allocList;

  bool allocated = DoNrSlAllocation (params, itDst->second, allocList);

  if (!allocated)
    {
      return;
    }
  m_nrSlUeMacSchedSapUser->SchedUeNrSlConfigInd (allocList);
}


uint8_t
NrSlUeMacSchedulerDefault::GetTotalSubCh () const
{
  return m_nrSlUeMacSchedSapUser->GetTotalSubCh ();
}

uint8_t
NrSlUeMacSchedulerDefault::GetSlMaxTxTransNumPssch () const
{
  return m_nrSlUeMacSchedSapUser->GetSlMaxTxTransNumPssch ();
}


void
NrSlUeMacSchedulerDefault::InstallNrSlAmc (const Ptr<NrAmc> &nrSlAmc)
{
  NS_LOG_FUNCTION (this);
  m_nrSlAmc = nrSlAmc;
  //In NR it does not have any impact
  m_nrSlAmc->SetUlMode ();
}

Ptr<const NrAmc>
NrSlUeMacSchedulerDefault::GetNrSlAmc () const
{
  NS_LOG_FUNCTION (this);
  return m_nrSlAmc;
}

void
NrSlUeMacSchedulerDefault::UseFixedNrSlMcs (bool fixMcs)
{
  NS_LOG_FUNCTION (this);
  m_fixedNrSlMcs = fixMcs;
}

bool
NrSlUeMacSchedulerDefault::IsNrSlMcsFixed () const
{
  NS_LOG_FUNCTION (this);
  return m_fixedNrSlMcs;
}

void
NrSlUeMacSchedulerDefault::SetInitialNrSlMcs (uint8_t mcs)
{
  NS_LOG_FUNCTION (this);
  m_initialNrSlMcs = mcs;
}

uint8_t
NrSlUeMacSchedulerDefault::GetInitialNrSlMcs () const
{
  NS_LOG_FUNCTION (this);
  return m_initialNrSlMcs;
}

uint8_t
NrSlUeMacSchedulerDefault::GetRv (uint8_t txNumTb) const
{
  NS_LOG_FUNCTION (this << +txNumTb);
  uint8_t modulo  = txNumTb % 4;
  //we assume rvid = 0, so RV would take 0, 2, 3, 1
  //see TS 38.21 table 6.1.2.1-2
  uint8_t rv = 0;
  switch (modulo)
  {
    case 0:
      rv = 0;
      break;
    case 1:
      rv = 2;
      break;
    case 2:
      rv = 3;
      break;
    case 3:
      rv = 1;
      break;
    default:
      NS_ABORT_MSG ("Wrong modulo result to deduce RV");
  }

  return rv;
}

int64_t
NrSlUeMacSchedulerDefault::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_uniformVariable->SetStream (stream );
  return 1;
}

bool
NrSlUeMacSchedulerDefault::DoNrSlAllocation (const std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>& txOpps,
                                            const std::shared_ptr<NrSlUeMacSchedulerDstInfo> &dstInfo,
                                            std::set<NrSlSlotAlloc> &slotAllocList)
{
  NS_LOG_FUNCTION (this);
  bool allocated = false;
  NS_ASSERT_MSG (txOpps.size () > 0, "Scheduler received an empty txOpps list from UE MAC");
  const auto & lcgMap = dstInfo->GetNrSlLCG (); //Map of unique_ptr should not copy

  NS_ASSERT_MSG (lcgMap.size () == 1, "NrSlUeMacSchedulerDefault can handle only one LCG");

  std::vector<uint8_t> lcVector = lcgMap.begin ()->second->GetLCId ();
  NS_ASSERT_MSG (lcVector.size () == 1, "NrSlUeMacSchedulerDefault can handle only one LC");

  uint32_t bufferSize = lcgMap.begin ()->second->GetTotalSizeOfLC (lcVector.at (0));

  if (bufferSize == 0)
    {
      return allocated;
    }

  NS_ASSERT_MSG (IsNrSlMcsFixed (), "Attribute FixNrSlMcs must be true for NrSlUeMacSchedulerDefault scheduler");


  std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> selectedTxOpps;
  selectedTxOpps = RandomlySelectSlots (txOpps);
  NS_ASSERT_MSG (selectedTxOpps.size () > 0, "Scheduler should select at least 1 slot from txOpps");
  uint32_t tbs = 0;
  uint8_t assignedSbCh = 0;
  uint16_t availableSymbols = selectedTxOpps.begin ()->slPsschSymLength;
  uint16_t sbChSize = selectedTxOpps.begin ()->slSubchannelSize;
  NS_LOG_DEBUG ("Total available symbols for PSSCH = " << availableSymbols);
  //find the minimum available number of contiguous sub-channels in the
  //selected TxOpps
  auto sbChInfo = GetAvailSbChInfo (selectedTxOpps);
  NS_ABORT_MSG_IF (sbChInfo.availSbChIndPerSlot.size () != selectedTxOpps.size (), "subChInfo vector does not have info for all the selected slots");
  do
    {
      assignedSbCh++;
      tbs = GetNrSlAmc ()->CalculateTbSize (dstInfo->GetDstMcs (), sbChSize * assignedSbCh * availableSymbols);
    }
  while (tbs < bufferSize + 5 /*(5 bytes overhead of SCI format 2A)*/ && (sbChInfo.numSubCh - assignedSbCh) > 0);

  //Now, before allocating bytes to LCs we subtract 5 bytes for SCI format 2A
  //since we already took it into account while computing the TB size.
  tbs = tbs - 5 /*(5 bytes overhead of SCI stage 2)*/;

  //NS_LOG_DEBUG ("number of allocated subchannles = " << +assignedSbCh);

  std::vector <uint8_t> startSubChIndexPerSlot = RandSelSbChStart (sbChInfo, assignedSbCh);

  allocated = true;

  auto itsbChIndexPerSlot = startSubChIndexPerSlot.cbegin ();
  auto itTxOpps = selectedTxOpps.cbegin ();

  for (; itTxOpps != selectedTxOpps.cend () && itsbChIndexPerSlot != startSubChIndexPerSlot.cend (); ++itTxOpps, ++itsbChIndexPerSlot)
    {
      NrSlSlotAlloc slotAlloc;
      slotAlloc.sfn = itTxOpps->sfn;
      slotAlloc.dstL2Id = dstInfo->GetDstL2Id ();
      slotAlloc.priority = lcgMap.begin ()->second->GetLcPriority (lcVector.at (0));
      SlRlcPduInfo slRlcPduInfo (lcVector.at (0), tbs);
      slotAlloc.slRlcPduInfo.push_back (slRlcPduInfo);
      slotAlloc.mcs = dstInfo->GetDstMcs ();
      //PSCCH
      slotAlloc.numSlPscchRbs = itTxOpps->numSlPscchRbs;
      slotAlloc.slPscchSymStart = itTxOpps->slPscchSymStart;
      slotAlloc.slPscchSymLength = itTxOpps->slPscchSymLength;
      //PSSCH
      slotAlloc.slPsschSymStart = itTxOpps->slPsschSymStart;
      slotAlloc.slPsschSymLength = availableSymbols;
      slotAlloc.slPsschSubChStart = *itsbChIndexPerSlot;
      slotAlloc.slPsschSubChLength = assignedSbCh;
      slotAlloc.maxNumPerReserve = itTxOpps->slMaxNumPerReserve;
      slotAlloc.ndi = slotAllocList.empty () == true ? 1 : 0;
      slotAlloc.rv = GetRv (static_cast<uint8_t>(slotAllocList.size ()));
      if (static_cast<uint16_t>(slotAllocList.size ()) % itTxOpps->slMaxNumPerReserve == 0)
        {
          slotAlloc.txSci1A = true;
          if (slotAllocList.size () + itTxOpps->slMaxNumPerReserve <= selectedTxOpps.size ())
            {
              slotAlloc.slotNumInd = itTxOpps->slMaxNumPerReserve;
            }
          else
            {
              slotAlloc.slotNumInd = selectedTxOpps.size () - slotAllocList.size ();
            }
        }
      else
        {
          slotAlloc.txSci1A = false;
          //Slot, which does not carry SCI 1-A can not indicate future TXs
          slotAlloc.slotNumInd = 0;
        }

      slotAllocList.emplace (slotAlloc);
    }

  lcgMap.begin ()->second->AssignedData (lcVector.at (0), tbs);
  return allocated;
}


std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>
NrSlUeMacSchedulerDefault::RandomlySelectSlots (std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> txOpps)
{
  NS_LOG_FUNCTION (this);

  uint8_t totalTx = GetSlMaxTxTransNumPssch ();
  std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> newTxOpps;

  if (txOpps.size () > totalTx)
    {
      while (newTxOpps.size () != totalTx)
        {
          auto txOppsIt = txOpps.begin ();
          // Walk through list until the random element is reached
          std::advance (txOppsIt, m_uniformVariable->GetInteger (0, txOpps.size () - 1));
          //copy the randomly selected slot info into the new list
          newTxOpps.emplace_back (*txOppsIt);
          //erase the selected one from the list
          txOppsIt = txOpps.erase (txOppsIt);
        }
    }
  else
    {
      newTxOpps = txOpps;
    }
  //sort the list by SfnSf before returning
  newTxOpps.sort ();
  NS_ASSERT_MSG (newTxOpps.size () <= totalTx, "Number of randomly selected slots exceeded total number of TX");
  return newTxOpps;
}

NrSlUeMacSchedulerDefault::SbChInfo
NrSlUeMacSchedulerDefault::GetAvailSbChInfo (std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> txOpps)
{
  NS_LOG_FUNCTION (this << txOpps.size ());
  //txOpps are the randomly selected slots for 1st Tx and possible ReTx
  SbChInfo info;
  info.numSubCh = GetTotalSubCh ();
  std::vector <std::vector<uint8_t>> availSbChIndPerSlot;
  for (const auto &it:txOpps)
    {
      std::vector<uint8_t> indexes;
      for (uint8_t i = 0; i < GetTotalSubCh(); i++)
        {
          auto it2 = it.occupiedSbCh.find (i);
          if (it2 == it.occupiedSbCh.end ())
            {
              //available subchannel index(s)
              indexes.push_back (i);
            }
        }
      //it may happen that all sub-channels are occupied
      //remember scheduler can get a slot with all the
      //subchannels occupied because of 3 dB RSRP threshold
      //at UE MAC
      if (indexes.size () == 0)
        {
          for (uint8_t i = 0; i < GetTotalSubCh(); i++)
            {
              indexes.push_back (i);
            }
        }

      NS_ABORT_MSG_IF (indexes.size () == 0, "Available subchannels are zero");

      availSbChIndPerSlot.push_back (indexes);
      uint8_t counter = 0;
      for (uint8_t i = 0; i < indexes.size (); i++)
        {
          uint8_t counter2 = 0;
          uint8_t j = i;
          do
            {
              j++;
              if (j != indexes.size ())
                {
                  counter2++;
                }
              else
                {
                  counter2++;
                  break;
                }
            }
          while (indexes.at (j) == indexes.at (j - 1) + 1);

          counter = std::max (counter, counter2);
        }

      info.numSubCh = std::min (counter, info.numSubCh);
    }

  info.availSbChIndPerSlot = availSbChIndPerSlot;
  for (const auto &it:info.availSbChIndPerSlot)
    {
      NS_ABORT_MSG_IF (it.size () == 0, "Available subchannel size is 0");
    }
  return info;
}

std::vector <uint8_t>
NrSlUeMacSchedulerDefault::RandSelSbChStart (SbChInfo sbChInfo, uint8_t assignedSbCh)
{
  NS_LOG_FUNCTION (this << +sbChInfo.numSubCh << sbChInfo.availSbChIndPerSlot.size () << +assignedSbCh);

  std::vector <uint8_t> subChInStartPerSlot;
  uint8_t minContgSbCh = sbChInfo.numSubCh;

  for (const auto &it:sbChInfo.availSbChIndPerSlot)
    {
      if (minContgSbCh == GetTotalSubCh() && assignedSbCh == 1)
        {
          //quick exit
          uint8_t randIndex = static_cast<uint8_t> (m_uniformVariable->GetInteger (0, GetTotalSubCh() - 1));
          subChInStartPerSlot.push_back (randIndex);
        }
      else
        {
          bool foundRandSbChStart = false;
          auto indexes = it;
          do
            {
              NS_ABORT_MSG_IF (indexes.size () == 0, "No subchannels available to choose from");
              uint8_t randIndex = static_cast<uint8_t> (m_uniformVariable->GetInteger (0, indexes.size () - 1));
              NS_LOG_DEBUG ("Randomly drawn index of the subchannel vector is " << +randIndex);
              uint8_t sbChCounter = 0;
              for (uint8_t i = randIndex; i < indexes.size (); i++)
                {
                  sbChCounter++;
                  auto it = std::find (indexes.begin(), indexes.end(), indexes.at (i) + 1);
                  if (sbChCounter == assignedSbCh)
                    {
                      foundRandSbChStart = true;
                      NS_LOG_DEBUG ("Random starting sbch is " << +indexes.at (randIndex));
                      subChInStartPerSlot.push_back (indexes.at (randIndex));
                      break;
                    }
                  if (it == indexes.end())
                    {
                      break;
                    }
                }
            }
          while (foundRandSbChStart == false);
        }
    }

  return subChInStartPerSlot;
}



} //namespace ns3
