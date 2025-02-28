modded class SCR_RespawnSystemComponent
{
	
	override int GetFactionPlayerCount(Faction faction)
	{
		PR_FactionMemberManager factionMemberManager = PR_FactionMemberManager.GetInstance();
		if(!factionMemberManager)
			return null;
		
		PR_RoleToPlayer members = factionMemberManager.GetFactionMembers(faction);
		if(members)
			return members.m_aPlayers.Count();
		else
			return 0;
	}

    override static Faction GetLocalPlayerFaction(IEntity player = null)
    {
		PR_FactionMemberManager factionMemberManager = PR_FactionMemberManager.GetInstance();
		if(!factionMemberManager)
			return null;
		
		int playerID = SCR_PlayerController.GetLocalPlayerId();
		return factionMemberManager.GetPlayerFaction(playerID);
    }
	
	override Faction GetPlayerFaction(int playerId)
	{
		PR_FactionMemberManager factionMemberManager = PR_FactionMemberManager.GetInstance();
		if(!factionMemberManager)
			return null;
		
		return factionMemberManager.GetPlayerFaction(playerId);
	}
	
	override void SetPlayerFaction(int playerId, int factionIndex)
	{
		PR_FactionMemberManager factionMemberManager = PR_FactionMemberManager.GetInstance();
		if(!factionMemberManager)
			return;
		
		factionMemberManager.SetPlayerFaction(factionIndex, playerId);
		
		if (m_pGameMode)
			m_pGameMode.HandleOnFactionAssigned(playerId, factionMemberManager.GetPlayerFaction(playerId));
	}
	
	//------------------------------------------------------------------------------------------------
	// Called from SCR_RespawnComponent
	override void DoSetPlayerFaction(int playerId, int factionIndex)
	{
		// Verify that faction makes sense
		if (!CanSetFaction(playerId, factionIndex))
		{
			ERespawnSelectionResult res = ERespawnSelectionResult.ERROR_FORBIDDEN;
			// In case no player controller is present, respawn component will not be present either,
			// can happen in case of disconnection
			SCR_RespawnComponent respawnComponent = SCR_RespawnComponent.Cast(GetGame().GetPlayerManager().GetPlayerRespawnComponent(playerId));
			if (respawnComponent)
				respawnComponent.AcknowledgePlayerFactionSet(res);
			return;
		}

		Faction faction = GetPlayerFaction(playerId);

		int oldIndex = GetFactionIndex(faction);
		if (oldIndex != factionIndex)
		{
			if (oldIndex != SCR_PlayerRespawnInfo.RESPAWN_INFO_INVALID_INDEX)
			{
				if (IsIntArrayIndexValid(m_aFactionPlayerCount, oldIndex))
				{
					m_aFactionPlayerCount[oldIndex] = m_aFactionPlayerCount[oldIndex]-1;
					Replication.BumpMe();
				}
			}

			if (IsIntArrayIndexValid(m_aFactionPlayerCount, factionIndex))
			{
				m_aFactionPlayerCount[factionIndex] = m_aFactionPlayerCount[factionIndex]+1;
				Replication.BumpMe();
			}
			else if (factionIndex != SCR_PlayerRespawnInfo.RESPAWN_INFO_INVALID_INDEX)
				Print("Provided Faction index in " + this.ToString() + " is out of array bounds!", LogLevel.ERROR);
		}

#ifdef RESPAWN_COMPONENT_VERBOSE
		Print("SCR_RespawnSystemComponent::DoSetPlayerFaction(playerId: "+playerId+", factionIndex: "+factionIndex+")");
#endif
		m_OnPlayerFactionChanged.Invoke(playerId, factionIndex);
		
		SetPlayerFaction(playerId, factionIndex);

		Replication.BumpMe();

		ERespawnSelectionResult res = ERespawnSelectionResult.OK;
		// In case no player controller is present, respawn component will not be present either,
		// can happen in case of disconnection
		SCR_RespawnComponent respawnComponent = SCR_RespawnComponent.Cast(GetGame().GetPlayerManager().GetPlayerRespawnComponent(playerId));
		if (respawnComponent)
			respawnComponent.AcknowledgePlayerFactionSet(res);
	}

	
}