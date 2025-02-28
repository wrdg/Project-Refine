modded class SCR_Faction
{
	[Attribute(desc: "List of PR assets related to this faction.")]
	protected ref PR_AssetList m_aAssetList;
	
	[Attribute(desc: "List of PR roles related to this faction.")]
	protected ref PR_RoleList m_aRoleList;
	
	[Attribute(desc: "List of PR building assets related to this faction.")]
	protected ref PR_BuildingEntryCategory m_aBuildingList;
	
	[Attribute(desc: "Dummy character entity to possess when in deploy menu", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et", category: "Entity Spawn Info")]
	protected ResourceName m_sDummyPrefab;
	
	[Attribute(desc: "Dummy character entity to possess when in deploy menu as SL", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et", category: "Entity Spawn Info")]
	protected ResourceName m_sRadioDummyPrefab;
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get the asset list assigned to this faction
	\return PR_AssetList
	*/
	PR_AssetList GetAssetList()
	{
		return m_aAssetList;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get the role list assigned to this faction
	\return PR_RoleList
	*/
	PR_RoleList GetRoleList()
	{
		return m_aRoleList;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get the role list assigned to this faction
	\return PR_RoleList
	*/
	PR_BuildingEntryCategory GetBuildingList()
	{
		return m_aBuildingList;
	}
	
	//------------------------------------------------------------------------------------------------
	// Returns dummy chracter prefab
	ResourceName GetDummyPrefab()
	{
		return m_sDummyPrefab;
	}
	
	ResourceName GetDummyRadioPrefab()
	{
		return m_sRadioDummyPrefab;
	}
	
	vector GetFactionDummySpawnPosition()
	{
		float id = GetId();
		vector offset = id * Vector(100, 0, 0);
		return offset;
	}
	
	//------------------------------------------------------------------------------------------------
	// Returns index of faction within FactionManager
	int GetId()
	{
		FactionManager fm = GetGame().GetFactionManager();
		return fm.GetFactionIndex(this);
	}
	
	//------------------------------------------------------------------------------------------------
	// Otherwise SCR_RespawnSystemComponent.CanSetFaction doesn't allow us to select faction
	override bool IsPlayable()
	{
		return true;
	}
}