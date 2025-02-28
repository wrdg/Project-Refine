[EntityEditorProps(category: "GameScripted/GameMode", description: "Universal map icon entity", visible: false)]
class PR_ActiveMapIconClass : SCR_PositionClass
{
};

//------------------------------------------------------------------------------------------------
//! Entity which follows its target and is drawn on the map via the SCR_MapDescriptorComponent
class PR_ActiveMapIcon : SCR_Position
{
	// How often to update position
	protected static const int POSITION_UPDATE_INTERVAL = 30;
	
	// Target entity which we follow
	protected IEntity m_Target;
	protected bool m_bTargetAssigned = false;
	
	protected MapDescriptorComponent m_MapDescriptor;
	
	protected int m_iRefreshCounter;
	
	//------------------------------------------------------------------------------------------------
	// RPL PROPS
	
	// Replicated vector which stores position without vertical component and direction
	// Format: [x pos, dir angle, z pos]
	// TODO: Make a custom struct & codec with limited precision to save traffic
	[RplProp(onRplName: "UpdateFromReplicatedState")]
	protected vector m_vPosAndDir;
	
	// Faction ID for which this icon is relevant. -1 means it's for all factions.
	[RplProp(onRplName: "OnRplFactionChanged")]
	int m_iFactionId = -1;
	
	// Group ID for which this icon is relevant. -1 means it's for all groups.
	[RplProp(onRplName: "UpdateFromReplicatedState")]
	protected int m_iGroupId = -1;
	
	//------------------------------------------------------------------------------------------------
	// ATTRIBUTES
	
	[Attribute(desc: "Style of the icon")]
	protected ref PR_MapIconStyleBase m_Style;
	
	protected vector prevTransform[4];
	protected bool firstMove = false;
	
	//------------------------------------------------------------------------------------------------
	// PUBLIC
	
	// Initializes this icon
	// Called only on server
	// If target is not null, we start tracking the target
	// If target is null, we just set position explicitly
	void Init(IEntity target, vector pos = vector.Zero, int factionId = -1, int groupId = -1)
	{
		m_Target = target;
		
		// We get position from target or set it explicitly
		if (target)
		{
			OnTargetAssigned(target);
			UpdatePosAndDirPropFromTarget();
			m_bTargetAssigned = true;
			
			// Subscribe to damange manager state change if exists
			ScriptedDamageManagerComponent damageManager = ScriptedDamageManagerComponent.Cast(m_Target.FindComponent(ScriptedDamageManagerComponent));
			if(damageManager)
			{
				damageManager.GetOnDamageStateChanged().Insert(OnDamageStateChanged);
			}
		}
		else
		{
			m_vPosAndDir[0] = pos[0];
			m_vPosAndDir[1] = 0;
			m_vPosAndDir[2] = pos[2];
		}
		
		// Faction affiliation
		m_iFactionId = factionId;
		if (m_Target)
		{
			FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(m_Target.FindComponent(FactionAffiliationComponent));
			UpdateFaction(factionAffiliation, true);
		}
		
		// TODO: Group affiliation
		m_iGroupId = groupId;
		
		UpdateVisibility();
		
		//  Initial setup I guess?
		UpdateFromReplicatedState();
		Replication.BumpMe();
		
		// Master will track target's state
		if (target)
		{
			// If we don't have a target to track then we don't need fixed frame
			SetEventMask(EntityEvent.FIXEDFRAME);
		}
	}
	
	void UpdateVisibility()
	{
		if(m_Style == null)
			return;

		if (PR_ActiveMapIconManagerComponent.GetInstance().CanStream(SCR_PlayerController.GetLocalPlayerId(), this))
		{
			m_Style.SetVisibility(true, m_MapDescriptor);
		}
		else
		{
			m_Style.SetVisibility(false, m_MapDescriptor);
		}
	}
	
	// Called when we hover a cursor on it
	void OnCursorHover(SCR_MapEntity mapEntity, SCR_MapCursorModule cursorModule);
	
	//------------------------------------------------------------------------------------------------
	// PROTECTED
	
	// Also called by replication when replicated state changes
	// This function must implement update of local state from replicated properties
	// Override this in inherited classes for custom functionality.
	// But remember to call this method of parent class!
	protected void UpdateFromReplicatedState()
	{
		SetTransformFromPosAndDirProp();
	}
	
	// On Client - FactionId of icon changed by Server
	protected void OnRplFactionChanged()
	{
		if (Replication.IsClient())
		{
			UpdateVisibility();
			UpdateFromReplicatedState();
		}
	}
	
	// Updates RPL prop member variables from target.
	// Override in inherited classes, but remember to call parent class method!
	protected void UpdatePropsFromTarget()
	{
		UpdatePosAndDirPropFromTarget();
	}
	
	// Here we can write custom logic which decides to which faction this icon is vibile
	// This is not meant to be called periodically, instead it is called from events
	// init - when true, we perform initialization, when false, this is called from faction change event
	protected void UpdateFaction(FactionAffiliationComponent factionAffiliationComponent, bool init)
	{
		if (!factionAffiliationComponent)
			return;
		
		// In generic case, the icon is visible to same faction as its target's current faction
		Faction affiliatedFaction = factionAffiliationComponent.GetAffiliatedFaction();
		FactionManager factionManager = GetGame().GetFactionManager();
		m_iFactionId = factionManager.GetFactionIndex(affiliatedFaction);
	}
	
	// Called once when a target is assigned.
	// Override to implement one-time initialization.
	protected void OnTargetAssigned(IEntity target);
	
	protected void OnDamageStateChanged(EDamageState state)
	{
		if (state == EDamageState.DESTROYED)
		{
			delete this;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{
		if (!GetGame().GetWorldEntity())
  			return;

		m_MapDescriptor = MapDescriptorComponent.Cast(owner.FindComponent(MapDescriptorComponent));
				
		RplComponent rpl = RplComponent.Cast(FindComponent(RplComponent));
		if (rpl && Replication.IsServer())
		{
			rpl.InsertToReplication();
		}
		
		if (m_Style)
		{
			m_Style.Apply(m_MapDescriptor);
		}	
		
		if(Replication.IsClient())
		{			
			PR_ActiveMapIconManagerComponent m_MapManager = PR_ActiveMapIconManagerComponent.GetInstance();
			if(m_MapManager)
			{
				m_MapManager.ClientRegister(this);
			}
		}
	}
	
	// Client player changed faction - hide / unhide this icon -> needed for now because things in the bubble of the player stay streamed in even though it might be from enemy team
	void OnPlayerFactionChanged(int playerID, int factionIndex)
	{
		if(SCR_PlayerController.GetLocalPlayerId() != playerID)
			return;
		
		UpdateVisibility();
	}
	
	//------------------------------------------------------------------------------------------------
	// Called when target changes faction.
	// !!! Works only on server!
	void OnTargetFactionChanged()
	{
		if (m_Target == null)
			return;
		
		SCR_FactionAffiliationComponent factionAffiliationComponent = SCR_FactionAffiliationComponent.Cast(m_Target.FindComponent(SCR_FactionAffiliationComponent));
		
		UpdateFaction(factionAffiliationComponent, false);
		
		Replication.BumpMe();
		
		UpdateVisibility();
	}
	
	//------------------------------------------------------------------------------------------------
	void PR_ActiveMapIcon(IEntitySource src, IEntity parent)
	{
		SetFlags(EntityFlags.ACTIVE, true);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~PR_ActiveMapIcon()
	{
		if(Replication.IsClient())
		{
			PR_ActiveMapIconManagerComponent m_MapManager = PR_ActiveMapIconManagerComponent.GetInstance();
			if(m_MapManager)
			{
				m_MapManager.Unregister(this);
			}
		}
	}
	
	// Converts target's XZ position and direction into one vector
	protected void UpdatePosAndDirPropFromTarget()
	{
		vector worldTransform[4];
		m_Target.GetWorldTransform(worldTransform);
		float dirAngle = Math.Atan2(worldTransform[2][0], worldTransform[2][2]); // Radians
		m_vPosAndDir = Vector(worldTransform[3][0], dirAngle, worldTransform[3][2]); // X, Dir, Z
	}
	
	// Sets this entity transform and map icon rotation from pos and dir vector
	protected void SetTransformFromPosAndDirProp()
	{
		// Set X,Z of this entity origin.
		// Don't set entity rotation from angle, we don't care where about this entity rotation
		vector newPos = m_vPosAndDir;
		newPos[1] = 0;
		SetOrigin(newPos);
		
		// But update marker direction
		if(!m_MapDescriptor)
			return;
		m_MapDescriptor.Item().SetAngle(Math.RAD2DEG * m_vPosAndDir[1]); 
	}
	
	override protected void EOnFixedFrame(IEntity owner, float timeSlice)
	{
		if(m_iRefreshCounter++ > POSITION_UPDATE_INTERVAL && m_bTargetAssigned)
		{
			if (m_Target)
			{
				if(!firstMove)
				{
					RplComponent rpl = RplComponent.Cast(owner.FindComponent(RplComponent));				
					rpl.ForceNodeMovement(prevTransform[3]);
				}
				firstMove = false;
				owner.GetWorldTransform(prevTransform);
				
				UpdatePropsFromTarget();
				Replication.BumpMe();
				UpdateFromReplicatedState();
			}
			else
			{
				// Target was deleted
				// TODO: delete ourselves			
			}
			
			m_iRefreshCounter = 0;
		}
	}
}