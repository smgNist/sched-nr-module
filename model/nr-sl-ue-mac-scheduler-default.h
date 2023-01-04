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
#ifndef NR_SL_UE_MAC_SCHEDULER_DEFAULT_H
#define NR_SL_UE_MAC_SCHEDULER_DEFAULT_H


#include "nr-sl-ue-mac-scheduler.h"
#include "nr-sl-ue-mac-scheduler-dst-info.h"
#include "nr-amc.h"
#include "nr-sl-phy-mac-common.h"
#include <ns3/random-variable-stream.h>
#include <memory>
#include <functional>
#include <list>

namespace ns3 {

/**
 * \ingroup scheduler
 *
 * \brief A general scheduler for NR SL UE in NS3
 */
class NrSlUeMacSchedulerDefault : public NrSlUeMacScheduler
{
public:
  /**
   * \brief GetTypeId
   *
   * \return The TypeId of the class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief NrSlUeMacSchedulerDefault default constructor
   */
  NrSlUeMacSchedulerDefault ();

  /**
   * \brief NrSlUeMacSchedulerDefault destructor
   */
  virtual ~NrSlUeMacSchedulerDefault ();

  /**
   * \brief Send the NR Sidelink logical channel configuration from UE MAC to the UE scheduler
   *
   * This method is also responsible to create the destination info.
   *
   * \see CreateDstInfo
   * \see CreateLCG
   * \see CreateLc
   *
   * \param params The NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo struct
   */
  virtual void DoCschedUeNrSlLcConfigReq (const NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo& params) override;

  /**
   * \brief UE RLC informs the scheduler of NR SL data
   *
   * The message contains the LC and the amount of data buffered. Therefore,
   * in this method we cycle through all the destinations LCG to find the LC, and once
   * it is found, it is updated with the new amount of data.
   *
   * \param params The buffer status report from UE RLC

   */
  virtual void DoSchedUeNrSlRlcBufferReq (const struct NrSlUeMacSchedSapProvider::SchedUeNrSlReportBufferStatusParams& params) override;
  /**
   * \brief Send NR Sidelink trigger request from UE MAC to the UE scheduler
   *
   * \param sfn The SfnSf
   * \param dstL2Id The destination layer 2 id
   * \param availableReso The list of NrSlUeMacSchedSapProvider::NrSlSlotInfo
   * \param ids available HARQ process IDs
   */
  virtual void DoSchedUeNrSlTriggerReq (const SfnSf& sfn, uint32_t dstL2Id, const std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>& availableReso, const std::deque<uint8_t>& ids) override;
  /**
   * \brief Attempt to select new grant from the selection window
   *
   * If successful, CreateSpsGrant () will be called for SPS grants
   * or CreateSinglePduGrant () for dynamic grants
   *
   * \param dstL2Id The destination layer 2 id
   * \param params The list of NrSlUeMacSchedSapProvider::NrSlSlotInfo
   * \param ids available HARQ process IDs
   */
  void AttemptGrantAllocation (uint32_t dstL2Id, const std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>& params, const std::deque<uint8_t>& ids);
  /**
   * \brief Create future SPS grants based on slot allocation
   *
   * \param slotAllocList The slot allocation list
   * \param rri The resource reservation interval
   * \return The grant info for a destination based on the scheduler allocation
   *
   * \see NrSlUeMacSchedSapUser::NrSlSlotAlloc
   * \see NrSlUeMacSchedSapUser::NrSlGrantInfo
   */
  NrSlUeMacSchedSapUser::NrSlGrantInfo CreateSpsGrantInfo (const std::set<NrSlSlotAlloc>& params, Time rri);
  /**
   * \brief Create a single-PDU grant based on slot allocation
   *
   * \param slotAllocList The slot allocation list
   * \return The grant info for a destination based on the scheduler allocation
   *
   * \see NrSlUeMacSchedSapUser::NrSlSlotAlloc
   * \see NrSlUeMacSchedSapUser::NrSlGrantInfo
   */
  NrSlUeMacSchedSapUser::NrSlGrantInfo CreateSinglePduGrantInfo (const std::set<NrSlSlotAlloc>& params);
  /**
   * \brief Filter the Transmit opportunities.
   *
   * Due to the semi-persistent scheduling, after calling the GetNrSlTxOpportunities
   * method, and before asking the scheduler for resources, we need to remove
   * those available slots, which are already part of the existing grant.
   *
   * \param txOppr The list of available slots
   * \return The list of slots which are not used by any existing semi-persistent grant.
   */
  std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> FilterTxOpportunities (std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> txOppr);
  /**
   * \brief Method to create future SPS grant repetitions
   * \param slotAllocList The slot allocation list from the selection window
   * \param ids The available HARQ process IDs
   * \param rri The resource reservation interval
   *
   * \see NrSlUeMacSchedSapUser::NrSlSlotAlloc
   */
  void CreateSpsGrant (const std::set<NrSlSlotAlloc>& slotAllocList, const std::deque<uint8_t>& ids, Time rri);
  /**
   * \brief Method to create a single-PDU grant
   * \param slotAllocList The slot allocation list from the selection window
   * \param ids The available HARQ process IDs
   *
   * \see NrSlUeMacSchedSapUser::NrSlSlotAlloc
   */
  void CreateSinglePduGrant (const std::set<NrSlSlotAlloc>& slotAllocList, const std::deque<uint8_t>& ids);

  /**
   * \brief Check whether any grants are at the processing delay deadline
   *        to send back to NrUeMac
   * \param sfn The current SfnSf
   */
  void CheckForGrantsToPublish (const SfnSf& sfn);

  /**
   * \brief Tell the scheduler that a new slot has started
   * XXX candidate for removal
   * \param sfn Ths current SfnSf
   * \param isSidelinkSlot Whether the slot is a sidelink slot
   */
  void DoSlotIndication (SfnSf sfn, bool isSidelinkSlot);

  /**
   * \brief Install the AMC for the NR Sidelink
   *
   * Usually called by the helper
   *
   * \param nrSlAmc NR Sidelink AMC
   */
  void InstallNrSlAmc (const Ptr<NrAmc> &nrSlAmc);

  /**
   * \brief Get the AMC for NR Sidelink
   *
   * \return the NR Sidelink AMC
   */
  Ptr<const NrAmc> GetNrSlAmc () const;

  /**
   * \brief Set the flag if the MCS for NR SL is fixed (in this case,
   *        it will take the initial value)
   *
   * \param fixMcs the flag to indicate if the NR SL MCS is fixed
   *
   * \see SetInitialMcsSl
   */
  void UseFixedNrSlMcs (bool fixMcs);
  /**
   * \brief Check if the MCS in NR SL is fixed
   * \return true if the NR SL MCS is fixed, false otherwise
   */
  bool IsNrSlMcsFixed () const;

  /**
   * \brief Set the initial value for the NR SL MCS
   *
   * \param mcs the MCS value
   */
  void SetInitialNrSlMcs (uint8_t mcs);

  /**
   * \brief Get the SL MCS initial value
   *
   * \return the value
   */
  uint8_t GetInitialNrSlMcs () const;

  /**
   * \brief Get Redundancy Version number
   *
   * We assume rvid = 0, so RV would take 0, 2, 3, 1. See TS 38.21 table 6.1.2.1-2
   *
   * \param txNumTb The transmission index of the TB, e.g., 0 for initial tx,
   *        1 for a first retransmission, and so on.
   * \return The Redundancy Version number
   */
  uint8_t GetRv (uint8_t txNumTb) const;

  /**
   * \brief Assign a fixed random variable stream number to the random variables
   * used by this model. Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream The first stream index to use
   * \return The number of stream indices assigned by this model
   */
  virtual int64_t AssignStreams (int64_t stream) override;


protected:
  /**
   * \brief Do the NE Sidelink allocation
   *
   * The SCI 1-A is Txed with every new transmission and after the transmission
   * for, which \c txNumTb mod MaxNumPerReserved == 0 \c , where the txNumTb
   * is the transmission index of the TB, e.g., 0 for initial tx, 1 for a first
   * retransmission, and so on.
   *
   * For allocating resources to more than one LCs of a
   * destination so they can be multiplexed, one could consider
   * the following procedure.
   *
   * 1. Irrespective of the priority of LCc, sum their buffer size.
   * 2. Compute the TB size using the AMC given the available resources, the
   *    buffer size computed in step 1, and the MCS.
   * 3. Starting from the highest priority LC, distribute the bytes among LCs
   *    from the TB size computed in step 2 as per their buffer status report
   *    until we satisfy all the LCs or the TB size computed in step 2 is fully
   *    consumed. There may be more than one LCs with the same priority, which
   *    could have same or different buffer sizes. In case of equal buffer sizes,
   *    these LCs should be assigned equal number of bytes. If these LCs have
   *    unequal buffer sizes, we can use the minimum buffer size among the LCs
   *    to assign the same bytes.
   *
   * \param params The list of the txOpps from the UE MAC
   * \param dstInfo The pointer to the NrSlUeMacSchedulerDstInfo of the destination
   *        for which UE MAC asked the scheduler to allocate the resourses
   * \param slotAllocList The slot allocation list to be updated by the scheduler
   * \return The status of the allocation, true if the destination has been
   *         allocated some resources; false otherwise.
   */
  virtual bool
  DoNrSlAllocation (const std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>& params,
                    const std::shared_ptr<NrSlUeMacSchedulerDstInfo> &dstInfo,
                    std::set<NrSlSlotAlloc> &slotAllocList);
  /**
   * \brief Method to get total number of sub-channels.
   *
   * \return the total number of sub-channels.
   */
  uint8_t GetTotalSubCh () const;

  /**
   * \brief Method to get the maximum transmission number
   *        (including new transmission and retransmission) for PSSCH.
   *
   * \return The max number of PSSCH transmissions
   */
  uint8_t GetSlMaxTxTransNumPssch () const;

  Ptr<UniformRandomVariable> m_uniformVariable; //!< Uniform random variable

private:
  /**
   * \brief Create destination info
   *
   * If the scheduler does not have the destination info then it creates it,
   * and then save its pointer in the m_dstMap map.
   *
   * If the scheduler already have the destination info, it does noting. This
   * could happen when we are trying add more than one logical channels
   * for a destination.
   *
   * \param params params of the UE
   * \return A std::shared_ptr to newly created NrSlUeMacSchedulerDstInfo
   */
  std::shared_ptr<NrSlUeMacSchedulerDstInfo>
  CreateDstInfo (const NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo& params);

  /**
   * \brief Create a NR Sidelink logical channel group
   *
   * A subclass can return its own representation of a logical channel by
   * implementing a proper subclass of NrSlUeMacSchedulerLCG and returning a
   * pointer to a newly created instance.
   *
   * \param lcGroup The logical channel group id
   * \return a pointer to the representation of a logical channel group
   */
  NrSlLCGPtr CreateLCG (uint8_t lcGroup) const;

  /**
   * \brief Create a NR Sidelink logical channel
   *
   * A subclass can return its own representation of a logical channel by
   * implementing a proper subclass of NrSlUeMacSchedulerLC and returning a
   * pointer to a newly created instance.
   *
   * \param params configuration of the logical channel
   * \return a pointer to the representation of a logical channel
   */

  NrSlLCPtr CreateLC (const NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo& params) const;

  /**
   * \ingroup scheduler
   * \brief The SbChInfo struct
   */
  struct SbChInfo
  {
    uint8_t numSubCh {0}; //!< The minimum number of contiguous subchannels that could be used for each slot.
    std::vector <std::vector<uint8_t>> availSbChIndPerSlot; //!< The vector containing the available subchannel index for each slot
  };
  /**
   * \brief Select the slots randomly from the available slots
   *
   * \param txOpps The list of the available TX opportunities
   * \return the set containing the indices of the randomly chosen slots in the
   *         txOpps list
   */
  std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>
  /**
   * \brief Randomly select the number of slots from the slots given by UE MAC
   *
   * If K denotes the total number of available slots, and N_PSSCH_maxTx is the
   * maximum number of PSSCH configured transmissions, then:
   *
   * N_Selected = N_PSSCH_maxTx , if K >= N_PSSCH_maxTx
   * otherwise;
   * N_Selected = K
   *
   * \param txOpps The list of the available slots
   * \return The list of randomly selected slots
   */
  RandomlySelectSlots (std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> txOpps);
  /**
   * \brief Get available subchannel information
   *
   * This method takes as input the randomly selected slots and computes the
   * maximum number of contiguous subchannels that are available for all
   * those slots. Moreover, it also returns the indexes of the available
   * subchannels for each slot.
   *
   * \param txOpps The list of randomly selected slots
   * \return A struct object of type SbChInfo
   */
  SbChInfo GetAvailSbChInfo (std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> txOpps);
  /**
   * \brief Randomly select the starting subchannel index
   *
   * This method, for each slot randomly selects the starting subchannel
   * index by taking into account the number of available contiguous subchannels
   * and the number of subchannels that needs to be assigned.
   *
   * \param sbChInfo A struct object of type SbChInfo
   * \param assignedSbCh The number of assigned subchannels
   * \return A vector containing the randomly chosen starting subchannel index
   *         for each slot.
   */
  std::vector <uint8_t> RandSelSbChStart (SbChInfo sbChInfo, uint8_t assignedSbCh);

  /**
   * \brief Get the random selection counter
   * \return The randomly selected reselection counter
   *
   * See 38.321 section 5.22.1.1 V16
   *
   * For 50 ms we use the range as per 36.321 section 5.14.1.1
   */
  uint8_t GetRandomReselectionCounter() const;
  /**
   * \brief Get the lower bound for the Sidelink resource re-selection
   *        counter when the resource reservation period is less than
   *        100 ms. It is as per the Change Request (CR) R2-2005970
   *        to TS 38.321.
   * \param pRsrv The resource reservation period
   * \return The lower bound of the range from which Sidelink resource re-selection
   *         counter will be drawn.
   */
  uint8_t GetLowerBoundReselCounter (uint16_t pRsrv) const;
  /**
   * \brief Get the upper bound for the Sidelink resource re-selection
   *        counter when the resource reservation period is less than
   *        100 ms. It is as per the Change Request (CR) R2-2005970
   *        to TS 38.321.
   * \param pRsrv The resource reservation period
   * \return The upper bound of the range from which Sidelink resource re-selection
   *         counter will be drawn.
   */
  uint8_t GetUpperBoundReselCounter (uint16_t pRsrv) const;

  std::unordered_map<uint32_t, std::shared_ptr<NrSlUeMacSchedulerDstInfo> > m_dstMap; //!< The map of between destination layer 2 id and the destination info

  Ptr<NrAmc> m_nrSlAmc;           //!< AMC pointer for NR SL

  bool    m_fixedNrSlMcs {false}; //!< Fixed MCS for *all* the destinations

  uint8_t m_initialNrSlMcs   {0}; //!< Initial (or fixed) value for NR SL MCS

  std::map<uint32_t, struct NrSlUeMacSchedSapUser::NrSlGrantInfo> m_grantInfo;
  Ptr<UniformRandomVariable> m_ueSelectedUniformVariable; //!< uniform random variable used for NR Sidelink
  double m_slProbResourceKeep {0.0}; //!< Sidelink probability of keeping a resource after resource re-selection counter reaches zero
  uint8_t m_reselCounter {0}; //!< The resource selection counter
  uint16_t m_cResel {0}; //!< The C_resel counter
  Time m_pRsvpTx {MilliSeconds (100)}; //!< Resource Reservation Interval for NR Sidelink in ms
  uint8_t m_t1 {2}; //!< The offset in number of slots between the slot in which the resource selection is triggered and the start of the selection window

};

} //namespace ns3

#endif /* NR_SL_UE_MAC_SCHEDULER_DEFAULT_H */
