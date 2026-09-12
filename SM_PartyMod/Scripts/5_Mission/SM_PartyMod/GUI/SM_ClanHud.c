class SM_ClanHud
{
	protected const int ENTRY_HEIGHT = 42;
	protected const int COMPACT_ENTRY_HEIGHT = 26;
	protected const int PANEL_WIDTH = 210;
	protected const int COMPACT_PANEL_WIDTH = 176;
	protected const float BAR_WIDTH = 190;
	protected const float COMPACT_BAR_WIDTH = 164;
	protected const float BAR_HEIGHT = 5;
	protected const float COMPACT_BAR_HEIGHT = 3;
	protected const float MARKER_WIDTH = 240;
	protected const float PING_WIDTH = 200;
	protected const float PING_ICON_ANCHOR_Y = 44;

	protected Widget m_Root;
	protected Widget m_Panel;
	protected ref array<Widget> m_EntryRoots = new array<Widget>;
	protected ref array<Widget> m_MarkerRoots = new array<Widget>;
	protected ref array<Widget> m_PingRoots = new array<Widget>;
	protected ref array<Widget> m_MapMarkerRoots = new array<Widget>;
	protected ref array<vector> m_MapMarker3DPos = new array<vector>;
	protected ref array<string> m_MapMarker3DName = new array<string>;
	protected ref array<int> m_MapMarker3DColor = new array<int>;
	protected ref array<string> m_MapMarker3DIcon = new array<string>;
	protected int m_VisibleCount;

	void Init()
	{
		m_Root = GetGame().GetWorkspace().CreateWidgets("SM_PartyMod/GUI/layouts/SM_ClanHud.layout");
		if (m_Root)
		{
			m_Panel = m_Root.FindAnyWidget("ClanHudPanel");
			m_Panel.Show(false);
			m_Root.Show(true);
		}
	}

	void Update(float timeslice)
	{
		if (!m_Root || !m_Panel)
			return;

		TickPings(timeslice);
		if (SM_HudVisibility.IsGameHudHidden())
		{
			m_Root.Show(false);
			return;
		}
		m_Root.Show(true);

		if (SM_ClanClientData.HudDirty)
		{
			SM_ClanClientData.HudDirty = false;
			Rebuild();
		}

		if (SM_ClanClientData.PingsDirty)
		{
			SM_ClanClientData.PingsDirty = false;
			RebuildPings();
		}

		if (SM_ClanClientData.MapMarkers3DDirty)
		{
			SM_ClanClientData.MapMarkers3DDirty = false;
			RebuildMapMarkers3D();
		}

		if (m_VisibleCount > 0 || m_PingRoots.Count() > 0 || m_MapMarkerRoots.Count() > 0)
			UpdateDynamic();
	}

	protected void TickPings(float timeslice)
	{
		for (int i = SM_ClanClientData.Pings.Count() - 1; i >= 0; i--)
		{
			SM_ClanClientData.Pings[i].TimeLeft -= timeslice;
			if (SM_ClanClientData.Pings[i].TimeLeft <= 0)
			{
				SM_ClanClientData.Pings.Remove(i);
				SM_ClanClientData.PingsDirty = true;
			}
		}
	}

	protected int GetEntryHeight()
	{
		if (SM_ClanClientData.HudCompact)
			return COMPACT_ENTRY_HEIGHT;
		return ENTRY_HEIGHT;
	}

	protected int GetPanelWidth()
	{
		if (SM_ClanClientData.HudCompact)
			return COMPACT_PANEL_WIDTH;
		return PANEL_WIDTH;
	}

	protected float GetBarWidth()
	{
		if (SM_ClanClientData.HudCompact)
			return COMPACT_BAR_WIDTH;
		return BAR_WIDTH;
	}

	protected float GetBarHeight()
	{
		if (SM_ClanClientData.HudCompact)
			return COMPACT_BAR_HEIGHT;
		return BAR_HEIGHT;
	}

	protected int ClampHudCoord(int value, int maxValue)
	{
		if (value < 0)
			return 0;
		if (value > maxValue)
			return maxValue;
		return value;
	}

	protected void GetSafeHudPosition(out int posX, out int posY)
	{
		int screenW;
		int screenH;
		GetScreenSize(screenW, screenH);

		int maxX = screenW - GetPanelWidth();
		if (maxX < 0)
			maxX = 0;

		int maxY = screenH - GetEntryHeight();
		if (maxY < 0)
			maxY = 0;

		posX = ClampHudCoord(SM_ClanClientData.HudOffsetX, maxX);
		posY = ClampHudCoord(SM_ClanClientData.HudOffsetY, maxY);
	}

	protected void ApplyEntryLayout(Widget entry, TextWidget nameText, TextWidget pctText, TextWidget statusText, TextWidget distText, Widget accent, Widget healthBg)
	{
		if (!entry)
			return;

		if (SM_ClanClientData.HudCompact)
		{
			entry.SetSize(COMPACT_PANEL_WIDTH, COMPACT_ENTRY_HEIGHT - 2);

			if (accent)
			{
				accent.SetPos(0, 0);
				accent.SetSize(2, COMPACT_ENTRY_HEIGHT - 2);
			}
			if (nameText)
			{
				nameText.SetPos(6, 1);
				nameText.SetSize(78, 14);
				nameText.SetTextExactSize(12);
			}
			if (distText)
			{
				distText.SetPos(86, 1);
				distText.SetSize(52, 14);
				distText.SetTextExactSize(10);
			}
			if (pctText)
			{
				pctText.SetPos(142, 1);
				pctText.SetSize(28, 14);
				pctText.SetTextExactSize(10);
			}
			if (healthBg)
			{
				healthBg.SetPos(6, 18);
				healthBg.SetSize(GetBarWidth(), GetBarHeight());
			}
			if (statusText)
			{
				statusText.Show(false);
				statusText.SetText(SM_PartyLoc.Text(""));
			}
			return;
		}

		entry.SetSize(PANEL_WIDTH, ENTRY_HEIGHT - 4);

		if (accent)
		{
			accent.SetPos(0, 0);
			accent.SetSize(3, ENTRY_HEIGHT - 4);
		}
		if (nameText)
		{
			nameText.SetPos(8, 2);
			nameText.SetSize(136, 15);
			nameText.SetTextExactSize(13);
		}
		if (pctText)
		{
			pctText.SetPos(148, 2);
			pctText.SetSize(54, 15);
			pctText.SetTextExactSize(12);
		}
		if (healthBg)
		{
			healthBg.SetPos(8, 19);
			healthBg.SetSize(GetBarWidth(), GetBarHeight());
		}
		if (statusText)
		{
			statusText.SetPos(8, 24);
			statusText.SetSize(132, 14);
			statusText.SetTextExactSize(10);
			statusText.Show(true);
		}
		if (distText)
		{
			distText.SetPos(142, 24);
			distText.SetSize(56, 14);
			distText.SetTextExactSize(10);
		}
	}

	protected void Rebuild()
	{
		m_VisibleCount = SM_ClanClientData.VisibleHudMembers.Count();
		if (m_VisibleCount > SM_ClanClientData.HudMaxVisibleMembers)
			m_VisibleCount = SM_ClanClientData.HudMaxVisibleMembers;

		if (!SM_ClanClientData.HudEnabled || !SM_ClanClientData.HasClan)
			m_VisibleCount = 0;

		SyncWidgetCount(m_EntryRoots, m_VisibleCount, "SM_PartyMod/GUI/layouts/SM_ClanHudEntry.layout", m_Panel);

		int markerCount = 0;
		if (SM_ClanClientData.MarkersEnabled && SM_ClanClientData.Local3DMarkersEnabled)
			markerCount = m_VisibleCount;
		SyncWidgetCount(m_MarkerRoots, markerCount, "SM_PartyMod/GUI/layouts/SM_ClanMarker.layout", m_Root);

		m_Panel.Show(m_VisibleCount > 0);
		if (m_VisibleCount == 0)
			return;

		int hudX;
		int hudY;
		GetSafeHudPosition(hudX, hudY);
		m_Panel.SetPos(hudX, hudY);
		m_Panel.SetSize(GetPanelWidth(), m_VisibleCount * GetEntryHeight());

		int clanColor = SM_ClanClientData.GetClanColor();
		float hudAlpha = SM_ClanClientData.HudOpacityPercent / 100.0;
		if (hudAlpha < 0)
			hudAlpha = 0;
		if (hudAlpha > 1)
			hudAlpha = 1;

		for (int i = 0; i < m_VisibleCount; i++)
		{
			SM_ClanMemberView member = SM_ClanClientData.VisibleHudMembers[i];
			Widget entry = m_EntryRoots[i];
			int entryHeight = GetEntryHeight();

			entry.SetPos(0, i * entryHeight);
			entry.SetAlpha(1.0);
			entry.SetColor(AlphaColor(110, ARGB(255, 0, 0, 0), hudAlpha));

			TextWidget nameText = TextWidget.Cast(entry.FindAnyWidget("MemberNameText"));
			TextWidget pctText = TextWidget.Cast(entry.FindAnyWidget("HealthPctText"));
			TextWidget statusText = TextWidget.Cast(entry.FindAnyWidget("StatusText"));
			TextWidget distText = TextWidget.Cast(entry.FindAnyWidget("DistanceText"));
			Widget accent = entry.FindAnyWidget("AccentStrip");
			Widget healthBg = entry.FindAnyWidget("HealthBarBg");
			Widget fill = entry.FindAnyWidget("HealthBarFill");

			ApplyEntryLayout(entry, nameText, pctText, statusText, distText, accent, healthBg);

			float hp = member.Health;
			if (hp < 0)
				hp = 0;
			if (hp > 1)
				hp = 1;

			if (nameText)
			{
				nameText.SetText(SM_PartyLoc.Text(member.Name));
				nameText.SetColor(AlphaColor(255, clanColor, hudAlpha));
			}
			if (accent)
				accent.SetColor(AlphaColor(255, clanColor, hudAlpha));
			if (pctText)
			{
				int hpPct = Math.Round(hp * 100);
				pctText.SetText(SM_PartyLoc.Text(hpPct.ToString() + "%"));
				pctText.SetColor(AlphaColor(255, ARGB(255, 212, 255, 199), hudAlpha));
			}
			if (distText)
				distText.SetColor(AlphaColor(255, ARGB(255, 212, 255, 199), hudAlpha));
			if (healthBg)
				healthBg.SetColor(AlphaColor(220, ARGB(255, 19, 26, 12), hudAlpha));

			if (fill)
			{
				float frac = hp;
				if (frac < 0.02 && hp > 0)
					frac = 0.02;
				fill.SetSize(frac, 1.0);

				int r = 220 - hp * 170;
				int g = 60 + hp * 160;
				fill.SetColor(ARGB(GetAlphaByte(255, hudAlpha), r, g, 50));
			}

			if (statusText)
			{
				string status = "";
				int statusColor = ARGB(255, 212, 255, 199);
				if (member.Status & SM_MemberStatus.UNCONSCIOUS)
				{
					status = "#STR_SMP_00276";
					statusColor = ARGB(255, 235, 70, 70);
				}
				else if (member.Status & SM_MemberStatus.BLEEDING)
				{
					status = "#STR_SMP_00589";
					statusColor = ARGB(255, 235, 150, 40);
				}
				else if (member.Status & SM_MemberStatus.IN_VEHICLE)
				{
					status = "#STR_SMP_00293";
				}
				else if (member.Status & SM_MemberStatus.TYPING_CHAT)
				{
					status = "#STR_SMP_00786";
				}
				else if (member.Status & SM_MemberStatus.VIEWING_MAP)
				{
					status = "#STR_SMP_00943";
				}
				statusText.SetText(SM_PartyLoc.Text(status));
				statusText.SetColor(AlphaColor(255, statusColor, hudAlpha));
			}

			if (i < m_MarkerRoots.Count())
			{
				TextWidget markerName = TextWidget.Cast(m_MarkerRoots[i].FindAnyWidget("MarkerNameText"));
				if (markerName)
				{
					markerName.SetText(SM_PartyLoc.Text(member.Name));
					markerName.SetColor(clanColor);
				}
			}
		}

		m_Panel.SetAlpha(1.0);
	}

	protected int GetAlphaByte(int baseAlpha, float opacity)
	{
		int alpha = Math.Round(baseAlpha * opacity);
		if (alpha < 0)
			alpha = 0;
		if (alpha > 255)
			alpha = 255;
		return alpha;
	}

	protected int AlphaColor(int baseAlpha, int color, float opacity)
	{
		int r = (color >> 16) & 0xFF;
		int g = (color >> 8) & 0xFF;
		int b = color & 0xFF;
		return ARGB(GetAlphaByte(baseAlpha, opacity), r, g, b);
	}

	protected void RebuildPings()
	{
		int count = SM_ClanClientData.Pings.Count();
		if (!SM_ClanClientData.HasClan || !SM_ClanClientData.PingsEnabled || !SM_ClanClientData.Local3DMarkersEnabled)
			count = 0;

		SyncWidgetCount(m_PingRoots, count, "SM_PartyMod/GUI/layouts/SM_ClanPingMarker.layout", m_Root);

		int clanColor = SM_ClanClientData.GetClanColor();
		for (int i = 0; i < count; i++)
		{
			SM_ClanPing ping = SM_ClanClientData.Pings[i];
			int pingColor = clanColor;
			string pingLabel = "";
			string pingIcon = SM_ClanMarkerIcon.GetPath();
			if (ping)
			{
				pingColor = ping.GetColor(clanColor);
				pingLabel = ping.GetHudLabel();
				pingIcon = ping.GetIconPath();
			}

			TextWidget author = TextWidget.Cast(m_PingRoots[i].FindAnyWidget("PingAuthorText"));
			if (author)
			{
				author.SetText(SM_PartyLoc.Text(pingLabel));
				author.SetColor(pingColor);
			}
			ApplyMarkerIconPath(m_PingRoots[i], pingColor, pingIcon);
		}
	}

	// Собирает 3D-метки карты — только те, что помечены индивидуально
	// (личные Show3D / выбранные клановые / выбранные серверные).
	protected void CollectMapMarkers3D()
	{
		m_MapMarker3DPos.Clear();
		m_MapMarker3DName.Clear();
		m_MapMarker3DColor.Clear();
		m_MapMarker3DIcon.Clear();

		if (!SM_ClanClientData.MapEnabled || !SM_ClanClientData.MarkersEnabled || !SM_ClanClientData.Local3DMarkersEnabled)
			return;

		SM_ClanClientData.EnsurePersonalMapMarkersLoaded();
		foreach (SM_PersonalMapMarker personalMarker : SM_ClanClientData.PersonalMapMarkers)
		{
			if (!personalMarker || !personalMarker.Show3D)
				continue;
			m_MapMarker3DPos.Insert(personalMarker.Position);
			m_MapMarker3DName.Insert(personalMarker.Name);
			m_MapMarker3DColor.Insert(SM_ClanColors.ResolveColor(personalMarker.Color));
			m_MapMarker3DIcon.Insert(SM_MapMarkerIconSet.GetPath(personalMarker.Icon));
		}

		if (SM_ClanClientData.HasClan)
		{
			foreach (SM_ClanMapMarker clanMarker : SM_ClanClientData.MapMarkers)
			{
				if (!clanMarker || !SM_ClanClientData.IsClanMarker3D(clanMarker.Id))
					continue;
				m_MapMarker3DPos.Insert(clanMarker.Position);
				m_MapMarker3DName.Insert(clanMarker.Name);
				m_MapMarker3DColor.Insert(SM_ClanColors.ResolveColor(clanMarker.Color));
				m_MapMarker3DIcon.Insert(SM_MapMarkerIconSet.GetPath(clanMarker.Icon));
			}
		}

		for (int s = 0; s < SM_ClanClientData.ServerMapMarkers.Count(); s++)
		{
			SM_ServerMapMarker serverMarker = SM_ClanClientData.ServerMapMarkers[s];
			if (!serverMarker || !SM_ClanClientData.IsServerMarker3DByIndex(s))
				continue;
			m_MapMarker3DPos.Insert(serverMarker.Position);
			m_MapMarker3DName.Insert(serverMarker.Name);
			m_MapMarker3DColor.Insert(SM_ClanColors.ResolveColor(serverMarker.GetColor(SM_ClanColors.GetColor(0))));
			string serverIcon = serverMarker.GetIconPath();
			m_MapMarker3DIcon.Insert(serverIcon);
		}
	}

	protected void RebuildMapMarkers3D()
	{
		CollectMapMarkers3D();
		int count = m_MapMarker3DPos.Count();

		SyncWidgetCount(m_MapMarkerRoots, count, "SM_PartyMod/GUI/layouts/SM_ClanPingMarker.layout", m_Root);

		for (int i = 0; i < count; i++)
		{
			int color = m_MapMarker3DColor[i];

			TextWidget nameText = TextWidget.Cast(m_MapMarkerRoots[i].FindAnyWidget("PingAuthorText"));
			if (nameText)
			{
				nameText.SetText(SM_PartyLoc.Text(m_MapMarker3DName[i]));
				nameText.SetColor(color);
			}
			string iconPath = SM_ClanMarkerIcon.GetPath();
			if (i < m_MapMarker3DIcon.Count())
				iconPath = m_MapMarker3DIcon[i];
			ApplyMarkerIconPath(m_MapMarkerRoots[i], color, iconPath);
		}
	}

	protected void ApplyMarkerIcon(Widget root, int color)
	{
		ApplyMarkerIconPath(root, color, SM_ClanMarkerIcon.GetPath());
	}

	protected void ApplyMarkerIconPath(Widget root, int color, string iconPath)
	{
		if (!root)
			return;

		iconPath = SM_MapMarkerIconPath.ForImageWidget(iconPath);

		ImageWidget icon = ImageWidget.Cast(root.FindAnyWidget("PingIcon"));
		if (icon)
		{
			icon.LoadImageFile(0, iconPath);
			icon.SetImage(0);
			icon.SetColor(color);
			icon.Show(true);
		}
	}

	protected void UpdateDynamic()
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;

		vector myPos = player.GetPosition();

		for (int i = 0; i < m_VisibleCount; i++)
		{
			SM_ClanMemberView member = SM_ClanClientData.VisibleHudMembers[i];
			vector memberPos = GetMemberWorldPosition(member);
			float distance = vector.Distance(myPos, memberPos);
			bool isSelf = false;
			if (member && member.Uid == SM_ClanClientData.MyUid)
				isSelf = true;

			if (i < m_EntryRoots.Count())
			{
				TextWidget distText = TextWidget.Cast(m_EntryRoots[i].FindAnyWidget("DistanceText"));
				if (distText)
				{
					string text = "";
					if (!isSelf)
					{
						if (SM_ClanClientData.ShowDistanceInHud)
							text = SM_PartyUtil.FormatDistance(distance);
						if (SM_ClanClientData.ShowDirectionInHud)
							text = text + " " + SM_PartyUtil.CompassDir(myPos, memberPos);
					}
					distText.SetText(SM_PartyLoc.Text(text));
				}
			}

			if (i < m_MarkerRoots.Count())
				PlaceWorldMarker(m_MarkerRoots[i], memberPos + "0 2.0 0", distance, MARKER_WIDTH, "MarkerDistText", SM_ClanClientData.MarkerMaxDistance, 3);
		}

		for (int p = 0; p < m_PingRoots.Count(); p++)
		{
			if (p >= SM_ClanClientData.Pings.Count())
			{
				m_PingRoots[p].Show(false);
				continue;
			}

			SM_ClanPing ping = SM_ClanClientData.Pings[p];
			float pingDist = vector.Distance(myPos, ping.Position);

			PlaceWorldMarker(m_PingRoots[p], ping.Position + "0 0.8 0", pingDist, PING_WIDTH, "PingDistText", SM_ClanClientData.PingMarkerMaxDistance, 0, PING_ICON_ANCHOR_Y);

			float alpha = 1.0;
			if (ping.TimeLeft < 5)
				alpha = 0.35 + 0.65 * Math.AbsFloat(Math.Sin(ping.TimeLeft * 4));
			m_PingRoots[p].SetAlpha(alpha);
		}

		for (int mm = 0; mm < m_MapMarkerRoots.Count(); mm++)
		{
			if (mm >= m_MapMarker3DPos.Count())
			{
				m_MapMarkerRoots[mm].Show(false);
				continue;
			}

			vector markerPos = m_MapMarker3DPos[mm];
			float markerDist = vector.Distance(myPos, markerPos);
			m_MapMarkerRoots[mm].SetAlpha(1.0);
			PlaceWorldMarker(m_MapMarkerRoots[mm], markerPos + "0 1.4 0", markerDist, PING_WIDTH, "PingDistText", 0, 0, PING_ICON_ANCHOR_Y);
		}
	}

	protected PlayerBase FindLocalMemberPlayer(string uid)
	{
		if (uid == "")
			return null;

		foreach (Man man : ClientData.m_PlayerBaseList)
		{
			PlayerBase pb = PlayerBase.Cast(man);
			if (!pb || !pb.GetIdentity())
				continue;
			if (!pb.IsAlive())
				continue;
			if (pb.GetIdentity().GetPlainId() == uid)
				return pb;
		}
		return null;
	}

	protected vector GetMemberWorldPosition(SM_ClanMemberView member)
	{
		if (!member)
			return "0 0 0";

		PlayerBase pb = FindLocalMemberPlayer(member.Uid);
		if (pb)
			return pb.GetPosition();
		return member.Position;
	}

	protected void PlaceWorldMarker(Widget marker, vector worldPos, float distance, float width, string distWidgetName, int maxDistance, float minDistance, float anchorOffsetY = 0)
	{
		bool visible = true;
		if (maxDistance > 0 && distance > maxDistance)
			visible = false;
		if (minDistance > 0 && distance < minDistance)
			visible = false;
		if (!IsWorldMarkerOnScreen(worldPos))
			visible = false;

		if (visible)
		{
			vector screenPos = GetGame().GetScreenPos(worldPos);
			if (screenPos[2] > 0)
			{
				marker.SetPos(Math.Round(screenPos[0] - width / 2), Math.Round(screenPos[1] - anchorOffsetY));

				TextWidget distText = TextWidget.Cast(marker.FindAnyWidget(distWidgetName));
				if (distText)
					distText.SetText(SM_PartyLoc.Text(SM_PartyUtil.FormatDistance(distance)));
			}
			else
			{
				visible = false;
			}
		}

		marker.Show(visible);
	}

	protected bool IsWorldMarkerOnScreen(vector worldPos)
	{
		vector relative = GetGame().GetScreenPosRelative(worldPos);
		if (relative[2] < 0)
			return false;
		if (relative[0] <= 0 || relative[0] >= 1)
			return false;
		if (relative[1] <= 0 || relative[1] >= 1)
			return false;
		return true;
	}

	protected void SyncWidgetCount(array<Widget> widgets, int target, string layoutPath, Widget parent)
	{
		while (widgets.Count() < target)
		{
			Widget newWidget = GetGame().GetWorkspace().CreateWidgets(layoutPath, parent);
			widgets.Insert(newWidget);
		}
		while (widgets.Count() > target)
		{
			int last = widgets.Count() - 1;
			widgets[last].Unlink();
			widgets.Remove(last);
		}
	}
}
