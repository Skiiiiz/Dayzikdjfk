modded class MissionBase
{
	override UIScriptedMenu CreateScriptedMenu(int id)
	{
		if (id == SM_PARTY_MENU_ID)
			return new SM_ClanMenu();
		if (id == SM_PARTY_CHAT_MENU_ID)
			return new SM_ClanChatMenu();

		return super.CreateScriptedMenu(id);
	}
}

modded class MissionGameplay
{
	protected ref SM_ClanHud m_SMClanHud;
	protected ref SM_ClanChatHud m_SMChatHud;
	protected bool m_SMStateRequested;
	protected float m_SMLastPingTime;
	protected float m_SMLastPingClearTime;
	protected bool m_SMChatScaleUpWasDown;
	protected bool m_SMChatScaleDownWasDown;
	protected bool m_SMChatHistoryUpWasDown;
	protected bool m_SMChatHistoryDownWasDown;
	protected bool m_SMChatClearedForJoin;
	protected int m_SMLastHudActivityStatus = -1;

	override void OnKeyPress(int key)
	{
		if (SM_HandleMenuKeyPress(key))
			return;

		super.OnKeyPress(key);
	}

	// Перехватываем игровой чат: все сообщения (игроки, сервер, [SERVER] от
	// других модов, киллфид) уводим в постоянное окно мода и подавляем
	// стандартный игровой чат, чтобы не было дублей и наложения.
	override void OnEvent(EventType eventTypeId, Param params)
	{
		if (eventTypeId == ChatMessageEventTypeID)
		{
			if (!SM_ClanClientData.ChatEnabled)
			{
				super.OnEvent(eventTypeId, params);
				return;
			}

			ChatMessageEventParams chatParams = ChatMessageEventParams.Cast(params);
			if (chatParams)
			{
				if (SM_CFToolsChat.ContainsTechnicalPrefix(chatParams.param2))
					return;
				if (SM_CFToolsChat.ContainsTechnicalPrefix(chatParams.param3))
					return;

				string label;
				int labelColor;
				int textColor;
				SM_VanillaChat.Classify(chatParams.param1, label, labelColor, textColor);
				string author = SM_VanillaChat.NormalizeAuthor(label, chatParams.param2);
				SM_ClanClientData.AddSystemChat(SM_ChatChannel.SERVER, label, author, chatParams.param3, labelColor, textColor);
			}
			return;
		}

		super.OnEvent(eventTypeId, params);
	}

	override void OnUpdate(float timeslice)
	{
		SM_VPPInputSuppressor.SuppressIfSMMenuOpen();

		Man preUpdatePlayer = GetGame().GetPlayer();
		if (preUpdatePlayer && !m_SMChatClearedForJoin)
		{
			m_SMChatClearedForJoin = true;
			SM_ClanClientData.ClearChatBuffer();
		}

		if (SM_CanOpenClanChat(preUpdatePlayer))
		{
			SM_OpenClanChat();
			return;
		}

		UAInput clanMenuInput = GetUApi().GetInputByName("UASMPClanMenu");
		if (clanMenuInput && clanMenuInput.LocalPress())
		{
			SM_ClanMenu openClanMenu = SM_ClanMenu.Cast(GetGame().GetUIManager().FindMenu(SM_PARTY_MENU_ID));
			if (openClanMenu)
			{
				if (!openClanMenu.SM_IsTextInputFocused())
				{
					SM_VPPInputSuppressor.SuppressVPPAdminInputs();
					openClanMenu.SM_CloseFromHotkey(false);
					return;
				}
			}

			if (preUpdatePlayer && !GetGame().GetUIManager().GetMenu())
			{
				PlayerBase menuPb = PlayerBase.Cast(preUpdatePlayer);
				if (menuPb && menuPb.IsAlive())
				{
					SM_VPPInputSuppressor.SuppressVPPAdminInputs();
					clanMenuInput.Supress();
					GetGame().GetUIManager().EnterScriptedMenu(SM_PARTY_MENU_ID, NULL);
					return;
				}
			}
		}

		UAInput clanMapInput = GetUApi().GetInputByName("UASMPClanMap");
		if (clanMapInput && clanMapInput.LocalPress())
		{
			SM_ClanMenu openMapMenu = SM_ClanMenu.Cast(GetGame().GetUIManager().FindMenu(SM_PARTY_MENU_ID));
			if (openMapMenu && openMapMenu.IsMapTabVisible())
			{
				if (!openMapMenu.SM_IsTextInputFocused())
				{
					SM_VPPInputSuppressor.SuppressVPPAdminInputs();
					openMapMenu.SM_CloseFromHotkey(false);
					return;
				}
			}

			if (preUpdatePlayer && SM_ClanClientData.MapEnabled && !GetGame().GetUIManager().GetMenu())
			{
				PlayerBase preMapPb = PlayerBase.Cast(preUpdatePlayer);
				if (preMapPb && preMapPb.IsAlive() && SM_ClanClientData.MapEnabled)
				{
					SM_VPPInputSuppressor.SuppressVPPAdminInputs();
					clanMapInput.Supress();
					SM_ClanMenu.RequestOpenMapFullscreenOnShow();
					GetGame().GetUIManager().EnterScriptedMenu(SM_PARTY_MENU_ID, NULL);
					return;
				}
			}
		}

		if (preUpdatePlayer && GetUApi().GetInputByName("UASMPToggle3DMarkers").LocalPress() && !GetGame().GetUIManager().GetMenu())
		{
			PlayerBase preTogglePb = PlayerBase.Cast(preUpdatePlayer);
			if (preTogglePb && preTogglePb.IsAlive())
			{
				bool enabled3D = SM_ClanClientData.ToggleLocal3DMarkers();
				if (enabled3D)
					SM_PartyLoc.AddNotification(2.0, "#STR_SMP_00505", "#STR_SMP_01110", "set:dayz_gui image:icon_info");
				else
					SM_PartyLoc.AddNotification(2.0, "#STR_SMP_00505", "#STR_SMP_01111", "set:dayz_gui image:icon_info");
				return;
			}
		}

		if (preUpdatePlayer && GetUApi().GetInputByName("UASMPClanPing").LocalPress() && !GetGame().GetUIManager().GetMenu())
		{
			PlayerBase prePingPb = PlayerBase.Cast(preUpdatePlayer);
			bool prePingAllowed = SM_ClanClientData.HasClan && SM_ClanClientData.PingsEnabled;
			if (prePingPb && prePingPb.IsAlive() && prePingAllowed)
			{
				SM_VPPInputSuppressor.SuppressVPPAdminInputs();
				SM_TrySendPing(prePingPb);
				return;
			}
		}

		if (preUpdatePlayer && GetUApi().GetInputByName("UASMPClanPingClear").LocalPress() && !GetGame().GetUIManager().GetMenu())
		{
			PlayerBase preClearPb = PlayerBase.Cast(preUpdatePlayer);
			bool preClearAllowed = SM_ClanClientData.HasClan && SM_ClanClientData.PingsEnabled;
			if (preClearPb && preClearPb.IsAlive() && preClearAllowed)
			{
				SM_VPPInputSuppressor.SuppressVPPAdminInputs();
				SM_TryClearPings(preClearPb);
				return;
			}
		}

		super.OnUpdate(timeslice);

		Man player = GetGame().GetPlayer();
		if (!player)
			return;

		if (!m_SMStateRequested)
		{
			m_SMStateRequested = true;
			ScriptRPC rpc = new ScriptRPC();
			rpc.Send(player, SM_PartyRPC.REQUEST_STATE, true, NULL);
		}

		if (!m_SMClanHud)
		{
			m_SMClanHud = new SM_ClanHud();
			m_SMClanHud.Init();
		}
		m_SMClanHud.Update(timeslice);

		if (!m_SMChatHud)
		{
			m_SMChatHud = new SM_ClanChatHud();
			m_SMChatHud.Init();
		}
		SM_UpdateClosedChatHotkeys();
		SM_UpdateHudActivityStatus(player);
		m_SMChatHud.Update(timeslice);

	}

	protected bool SM_CanOpenClanChat(Man player)
	{
		if (!player)
			return false;
		if (!GetUApi().GetInputByID(UAChat).LocalPress())
			return false;
		if (!SM_ClanClientData.ChatEnabled)
			return false;
		if (!SM_ClanClientData.HasClan && !SM_ClanClientData.LocalChatEnabled && !SM_ClanClientData.GlobalChatEnabled && !SM_ClanClientData.IsAdmin)
			return false;
		if (!GetGame().GetInput().IsEnabledMouseAndKeyboardEvenOnServer())
			return false;
		if (GetGame().GetUIManager().GetMenu())
			return false;

		PlayerBase pb = PlayerBase.Cast(player);
		if (!pb)
			return false;
		if (!pb.IsAlive())
			return false;
		if (pb.IsUnconscious())
			return false;

		return true;
	}

	protected bool SM_HandleMenuKeyPress(int key)
	{
		if (!GetGame() || !GetGame().GetUIManager())
			return false;

		SM_ClanMenu clanMenu = SM_ClanMenu.Cast(GetGame().GetUIManager().FindMenu(SM_PARTY_MENU_ID));
		if (clanMenu)
		{
			if (key == KeyCode.KC_ESCAPE)
			{
				clanMenu.SM_CloseFromHotkey(true);
				return true;
			}

			if (key == KeyCode.KC_P)
			{
				if (clanMenu.SM_IsTextInputFocused())
					return false;

				clanMenu.SM_CloseFromHotkey(false);
				return true;
			}

			if (key == KeyCode.KC_M)
			{
				if (!clanMenu.IsMapTabVisible())
					return false;

				if (clanMenu.SM_IsTextInputFocused())
					return false;

				clanMenu.SM_CloseFromHotkey(false);
				return true;
			}
		}

		SM_ClanChatMenu chatMenu = SM_ClanChatMenu.Cast(GetGame().GetUIManager().FindMenu(SM_PARTY_CHAT_MENU_ID));
		if (chatMenu && key == KeyCode.KC_ESCAPE)
		{
			chatMenu.Close();
			return true;
		}

		return false;
	}

	protected void SM_OpenClanChat()
	{
		SM_VPPInputSuppressor.SuppressVPPAdminInputs();
		GetGame().GetUIManager().EnterScriptedMenu(SM_PARTY_CHAT_MENU_ID, NULL);
	}

	protected void SM_UpdateHudActivityStatus(Man player)
	{
		int status = SM_GetHudActivityStatus();
		if (status == m_SMLastHudActivityStatus)
			return;

		m_SMLastHudActivityStatus = status;
		if (!player)
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(status);
		rpc.Send(player, SM_PartyRPC.HUD_ACTIVITY_STATUS, true, NULL);
	}

	protected int SM_GetHudActivityStatus()
	{
		int status = 0;
		if (!GetGame() || !GetGame().GetUIManager())
			return status;

		if (GetGame().GetUIManager().FindMenu(SM_PARTY_CHAT_MENU_ID))
			status |= SM_MemberStatus.TYPING_CHAT;
		else if (GetGame().GetUIManager().FindMenu(MENU_CHAT_INPUT))
			status |= SM_MemberStatus.TYPING_CHAT;

		if (GetGame().GetUIManager().FindMenu(MENU_MAP))
			status |= SM_MemberStatus.VIEWING_MAP;

		SM_ClanMenu clanMenu = SM_ClanMenu.Cast(GetGame().GetUIManager().FindMenu(SM_PARTY_MENU_ID));
		if (clanMenu && clanMenu.IsMapTabVisible())
			status |= SM_MemberStatus.VIEWING_MAP;

		return status;
	}

	protected void SM_UpdateClosedChatHotkeys()
	{
		if (!SM_ClanClientData.ChatEnabled)
		{
			m_SMChatScaleUpWasDown = false;
			m_SMChatScaleDownWasDown = false;
			m_SMChatHistoryUpWasDown = false;
			m_SMChatHistoryDownWasDown = false;
			return;
		}

		if (GetGame().GetUIManager().GetMenu())
		{
			m_SMChatScaleUpWasDown = false;
			m_SMChatScaleDownWasDown = false;
			m_SMChatHistoryUpWasDown = false;
			m_SMChatHistoryDownWasDown = false;
			return;
		}

		bool historyUpDown = KeyState(KeyCode.KC_PRIOR) != 0;
		if (historyUpDown && !m_SMChatHistoryUpWasDown)
			SM_ClanClientData.ScrollChatHistory(1);
		m_SMChatHistoryUpWasDown = historyUpDown;

		bool historyDownDown = KeyState(KeyCode.KC_NEXT) != 0;
		if (historyDownDown && !m_SMChatHistoryDownWasDown)
			SM_ClanClientData.ScrollChatHistory(-1);
		m_SMChatHistoryDownWasDown = historyDownDown;

		bool scaleUpDown = SM_IsCtrlDown() && SM_IsChatScaleUpPressed();
		if (scaleUpDown && !m_SMChatScaleUpWasDown)
			SM_ClanClientData.ChangeChatScale(SM_ClanClientData.ChatScaleStepPercent);
		m_SMChatScaleUpWasDown = scaleUpDown;

		bool scaleDownDown = SM_IsCtrlDown() && SM_IsChatScaleDownPressed();
		if (scaleDownDown && !m_SMChatScaleDownWasDown)
			SM_ClanClientData.ChangeChatScale(-SM_ClanClientData.ChatScaleStepPercent);
		m_SMChatScaleDownWasDown = scaleDownDown;
	}

	protected bool SM_IsCtrlDown()
	{
		if (KeyState(KeyCode.KC_LCONTROL) != 0)
			return true;
		if (KeyState(KeyCode.KC_RCONTROL) != 0)
			return true;
		return false;
	}

	protected bool SM_IsChatScaleUpPressed()
	{
		if (KeyState(KeyCode.KC_ADD) != 0)
			return true;
		if (KeyState(KeyCode.KC_EQUALS) != 0)
			return true;
		return false;
	}

	protected bool SM_IsChatScaleDownPressed()
	{
		if (KeyState(KeyCode.KC_SUBTRACT) != 0)
			return true;
		if (KeyState(KeyCode.KC_MINUS) != 0)
			return true;
		return false;
	}

	protected void SM_TrySendPing(PlayerBase player)
	{
		float now = GetGame().GetTickTime();
		if (now - m_SMLastPingTime < 0.7)
			return;

		vector rayDir = GetGame().GetCurrentCameraDirection();
		if (rayDir.Length() <= 0)
			return;

		// Как в SchanaParty: начинаем луч чуть перед камерой и ведем по направлению камеры.
		vector rayStart = GetGame().GetCurrentCameraPosition() + rayDir;
		vector rayEnd = rayStart + rayDir * 8000;
		vector hitPos;
		vector hitNormal;
		int hitComponent;
		set<Object> hitObjects = new set<Object>;

		if (!DayZPhysics.RaycastRV(rayStart, rayEnd, hitPos, hitNormal, hitComponent, hitObjects, NULL, player, true))
		{
			SM_ClanClientData.AddSystemChat(SM_ChatChannel.SERVER, "#STR_SMP_00639", "", "#STR_SMP_01057", ARGB(255, 248, 205, 115), ARGB(255, 239, 230, 218));
			return;
		}

		PlayerBase pingCorpse = SM_FindPingCorpse(hitObjects);
		if (pingCorpse)
		{
			string corpseName = SM_GetPingCorpseName(pingCorpse);
			if (corpseName != "")
			{
				m_SMLastPingTime = now;
				SM_SendObjectPing(player, pingCorpse.GetPosition(), corpseName, SM_ClanPingTargetType.CORPSE);
				return;
			}
		}

		Object pingVehicle = SM_FindPingVehicle(hitObjects);
		if (pingVehicle)
		{
			string vehicleName = SM_GetPingObjectName(pingVehicle);
			if (vehicleName != "")
			{
				m_SMLastPingTime = now;
				SM_SendObjectPing(player, pingVehicle.GetPosition(), vehicleName, SM_ClanPingTargetType.VEHICLE);
				return;
			}
		}

		ItemBase pingItem = SM_FindPingItem(hitObjects);
		if (pingItem)
		{
			string itemName = SM_GetPingItemName(pingItem);
			if (itemName != "")
			{
				m_SMLastPingTime = now;
				SM_SendObjectPing(player, pingItem.GetPosition(), itemName, SM_ClanPingTargetType.ITEM);
				return;
			}
		}

		m_SMLastPingTime = now;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(hitPos);
		rpc.Send(player, SM_PartyRPC.MARK_PLACE, true, NULL);
	}

	protected void SM_SendObjectPing(PlayerBase player, vector position, string objectName, int pingType)
	{
		ScriptRPC itemRpc = new ScriptRPC();
		itemRpc.Write(position);
		itemRpc.Write(objectName);
		itemRpc.Write(pingType);
		itemRpc.Send(player, SM_PartyRPC.MARK_ITEM, true, NULL);
	}

	protected bool SM_IsValidPingCorpse(PlayerBase corpse)
	{
		if (!corpse)
			return false;
		if (corpse.IsAlive())
			return false;
		return true;
	}

	protected PlayerBase SM_GetPingCorpseFromObject(Object obj)
	{
		PlayerBase corpse = PlayerBase.Cast(obj);
		if (SM_IsValidPingCorpse(corpse))
			return corpse;

		EntityAI entity = EntityAI.Cast(obj);
		if (!entity)
			return null;

		Man rootMan = entity.GetHierarchyRootPlayer();
		corpse = PlayerBase.Cast(rootMan);
		if (SM_IsValidPingCorpse(corpse))
			return corpse;

		return null;
	}

	protected PlayerBase SM_FindPingCorpse(set<Object> hitObjects)
	{
		if (!hitObjects)
			return null;

		for (int i = 0; i < hitObjects.Count(); i++)
		{
			PlayerBase corpse = SM_GetPingCorpseFromObject(hitObjects[i]);
			if (corpse)
				return corpse;
		}

		return null;
	}

	protected Object SM_GetPingVehicleFromObject(Object obj)
	{
		if (!obj)
			return null;
		if (obj.IsKindOf("Transport"))
			return obj;

		EntityAI entity = EntityAI.Cast(obj);
		if (!entity)
			return null;

		EntityAI root = entity.GetHierarchyRoot();
		if (root && root.IsKindOf("Transport"))
			return root;

		return null;
	}

	protected Object SM_FindPingVehicle(set<Object> hitObjects)
	{
		if (!hitObjects)
			return null;

		for (int i = 0; i < hitObjects.Count(); i++)
		{
			Object vehicle = SM_GetPingVehicleFromObject(hitObjects[i]);
			if (vehicle)
				return vehicle;
		}

		return null;
	}

	protected bool SM_IsValidPingItem(ItemBase item)
	{
		if (!item)
			return false;

		PlayerBase ownerPlayer = PlayerBase.Cast(item.GetHierarchyRootPlayer());
		if (ownerPlayer)
			return false;

		return true;
	}

	protected ItemBase SM_FindPingItem(set<Object> hitObjects)
	{
		if (!hitObjects)
			return null;

		for (int i = 0; i < hitObjects.Count(); i++)
		{
			Object obj = hitObjects[i];
			ItemBase item = ItemBase.Cast(obj);
			if (SM_IsValidPingItem(item))
				return item;
		}

		return null;
	}

	protected string SM_GetPingObjectName(Object obj)
	{
		if (!obj)
			return "";

		string objectName = obj.GetDisplayName();
		objectName = objectName.Trim();
		if (objectName == "")
			objectName = obj.GetType();
		return objectName;
	}

	protected string SM_GetPingCorpseName(PlayerBase corpse)
	{
		if (!corpse)
			return "";

		string corpseName = "";
		if (corpse.GetIdentity())
			corpseName = corpse.GetIdentity().GetName();
		corpseName = corpseName.Trim();
		if (corpseName == "")
			corpseName = corpse.GetDisplayName();
		corpseName = corpseName.Trim();
		if (corpseName == "")
			corpseName = "#STR_SMP_01108";
		return corpseName;
	}

	protected string SM_GetPingItemName(ItemBase item)
	{
		if (!item)
			return "";

		string itemName = SM_PartyUtil.GetItemDisplayName(item.GetType());
		itemName = itemName.Trim();
		if (itemName == "")
			itemName = item.GetType();
		return itemName;
	}

	protected void SM_TryClearPings(PlayerBase player)
	{
		float now = GetGame().GetTickTime();
		if (now - m_SMLastPingClearTime < 1.0)
			return;

		m_SMLastPingClearTime = now;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Send(player, SM_PartyRPC.MARK_CLEAR, true, NULL);
	}
}
