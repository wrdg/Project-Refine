/*
Component for one group entry with list of players
*/

class PR_GroupEntryComponent : ScriptedWidgetComponent
{
	protected ref PR_GroupEntryWidgets widgets = new PR_GroupEntryWidgets();
	
	protected Widget m_wRoot;
	
	protected SCR_AIGroup m_Group;
	
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		widgets.Init(w);
		
		widgets.m_ExpandButtonComponent.m_OnToggled.Insert(OnExpandButton);
		widgets.m_JoinLeaveButtonComponent.m_OnClicked.Insert(OnJoinLeaveButton);
		
		ExpandMemberList(GetExpanded());
		
		UpdateJoinLeaveButton();
	}
	
	override void HandlerDeattached(Widget w)
	{
		SCR_AIGroup.GetOnPlayerAdded().Remove(Event_OnPlayerAdded);
		SCR_AIGroup.GetOnPlayerRemoved().Remove(Event_OnPlayerRemoved);
		SCR_AIGroup.GetOnCustomNameChanged().Remove(Event_OnCustomNameChanged);
	}
	
	void Init(SCR_AIGroup group)
	{
		m_Group = group;
		
		SCR_AIGroup.GetOnPlayerAdded().Insert(Event_OnPlayerAdded);
		SCR_AIGroup.GetOnPlayerRemoved().Insert(Event_OnPlayerRemoved);
		SCR_AIGroup.GetOnCustomNameChanged().Insert(Event_OnCustomNameChanged);
		
		UpdateJoinLeaveButton();
	}
	
	void CreateGroupMemberLine(int playerId)
	{
		Widget w = GetGame().GetWorkspace().CreateWidgets(PR_GroupMemberLineWidgets.s_sLayout, widgets.m_GroupMemberList);
		PR_GroupMemberLineComponent grpMemberComp = PR_GroupMemberLineComponent.Cast(w.FindHandler(PR_GroupMemberLineComponent));
		grpMemberComp.Init(playerId);
	}
	
	PR_GroupMemberLineComponent FindGroupMemberLine(int playerId)
	{
		Widget wChild = widgets.m_GroupMemberList.GetChildren();
		while (wChild)
		{
			PR_GroupMemberLineComponent comp = PR_GroupMemberLineComponent.Cast(wChild.FindHandler(PR_GroupMemberLineComponent));
			if (comp.GetPlayerId() == playerId)
				return comp;
			wChild = wChild.GetSibling();
		}
		
		return null;
	}
	
	// Removes the group entry from the UI
	// It's done this way in case we want to customize the removal, via an animation or similar
	void RemoveFromUi()
	{
		m_wRoot.RemoveFromHierarchy();
	}
	
	// Updates state of join button depending on our state
	void UpdateJoinLeaveButton()
	{
		if (!m_Group)
		{
			widgets.m_JoinLeaveButtonComponent.SetEffectsWithAnyTagEnabled({"all", "not_joined"});
			return;
		}
		
		string buttonMode;
		
		if (m_Group.IsPlayerInGroup(GetGame().GetPlayerController().GetPlayerId())) // Are we in that group?
			buttonMode = "joined";
		else
			buttonMode = "not_joined";
		
		widgets.m_JoinLeaveButtonComponent.SetEffectsWithAnyTagEnabled({"all", buttonMode});
	}
	
	//-----------------------------------------------------------------------------------------------
	// Collapsing/expanding
	
	bool GetExpanded()
	{
		return widgets.m_ExpandButtonComponent.GetToggled();
	}
	
	protected void ExpandMemberList(bool expand)
	{
		widgets.m_GroupMemberList.SetVisible(expand);
	}
	
	//--------------------------------------------------------------------------------
	// Our UI events
	
	void OnJoinLeaveButton()
	{
		if (!m_Group)
			return;
		
		SCR_PlayerControllerGroupComponent groupComp = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		
		// Are we joining or leaving?
		if (m_Group.IsPlayerInGroup(GetGame().GetPlayerController().GetPlayerId()))
		{
			// Leave
			groupComp.RequestLeaveGroup();
		}
		else
		{
			// Join
			groupComp.RequestJoinGroup(m_Group.GetGroupID());
		}
	}
	
	protected void OnExpandButton()
	{
		ExpandMemberList(GetExpanded());
	}
	
	//--------------------------------------------------------------------------------
	// External events
	
	void Event_OnPlayerAdded(SCR_AIGroup group, int playerId)
	{
		// Bail if not our group
		if (group != m_Group)
			return;
		
		CreateGroupMemberLine(playerId);
		
		// Debugging
		//CreateGroupMemberLine(playerId);
		//CreateGroupMemberLine(playerId);
		//CreateGroupMemberLine(playerId);
		
		UpdateJoinLeaveButton();
	}
	
	void Event_OnPlayerRemoved(SCR_AIGroup group, int playerId)
	{
		// Bail if not our group
		if (group != m_Group)
			return;
		
		PR_GroupMemberLineComponent comp = FindGroupMemberLine(playerId);
		if (!comp)
			return; // wtf?
		
		comp.RemoveFromUi();
		
		UpdateJoinLeaveButton();
	}
	
	void Event_OnCustomNameChanged()
	{
		// Surprise surprise! We don't get new group name here, nor the group pointer
		// Just update our name anyway
		
		if (m_Group)
			widgets.m_GroupNameText.SetText(m_Group.GetCustomName());
	}
	
	//--------------------------------------------------------------------------------
	// Other minor things
	
	Widget GetRootWidget()
	{
		return m_wRoot;
	}
	
	SCR_AIGroup GetGroup()
	{
		return m_Group;
	}
}