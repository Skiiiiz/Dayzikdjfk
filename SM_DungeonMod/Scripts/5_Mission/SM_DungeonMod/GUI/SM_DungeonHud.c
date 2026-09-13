// ============================================================================
// SM_DungeonHud.c
//
// Пассивный HUD (без кнопок): текст фазы забега/обратного отсчёта, полоса
// прогресса эвакуации и короткие тосты-уведомления. Создаётся один раз в
// modded MissionGameplay и обновляется каждый кадр независимо от того,
// открыт ли SM_DungeonMenu.
// ============================================================================

class SM_DungeonHud
{
	protected Widget m_Root;
	protected TextWidget m_RunText;
	protected Widget m_EvacBarBg;
	protected Widget m_EvacBarFill;
	protected TextWidget m_NotifyText;

	protected float m_RunTextHideTimer;
	protected float m_NotifyHideTimer;
	protected string m_LastSeenNotify;

	void Init()
	{
		m_Root = GetGame().GetWorkspace().CreateWidgets("SM_DungeonMod/GUI/layouts/SM_DungeonHud.layout");
		if (!m_Root)
			return;

		m_RunText = TextWidget.Cast(m_Root.FindAnyWidget("SMDHudRunText"));
		m_EvacBarBg = m_Root.FindAnyWidget("SMDHudEvacBarBg");
		m_EvacBarFill = m_Root.FindAnyWidget("SMDHudEvacBarFill");
		m_NotifyText = TextWidget.Cast(m_Root.FindAnyWidget("SMDHudNotifyText"));

		m_Root.Show(true);
	}

	void Update(float timeslice)
	{
		if (!m_Root)
			return;

		if (SM_DungeonClientData.RunNotifyDirty)
		{
			SM_DungeonClientData.RunNotifyDirty = false;
			UpdateRunText();
		}
		if (SM_DungeonClientData.EvacDirty)
		{
			SM_DungeonClientData.EvacDirty = false;
			UpdateEvacBar();
		}
		if (SM_DungeonClientData.LastNotify != m_LastSeenNotify)
		{
			m_LastSeenNotify = SM_DungeonClientData.LastNotify;
			ShowNotify(m_LastSeenNotify);
		}

		if (m_RunTextHideTimer > 0)
		{
			m_RunTextHideTimer -= timeslice;
			if (m_RunTextHideTimer <= 0)
				m_RunText.Show(false);
		}
		if (m_NotifyHideTimer > 0)
		{
			m_NotifyHideTimer -= timeslice;
			if (m_NotifyHideTimer <= 0)
				m_NotifyText.Show(false);
		}
	}

	protected void UpdateRunText()
	{
		int phase = SM_DungeonClientData.RunPhase;
		string label = SM_DungeonRunPhase.ToLabel(phase);
		if (label == "")
		{
			m_RunText.Show(false);
			return;
		}

		string text = label;
		if (phase == SM_DungeonRunPhase.COUNTDOWN)
			text = label + " " + SM_DungeonClientData.RunSecondsRemaining.ToString();

		m_RunText.SetText(text);
		m_RunText.Show(true);

		if (phase == SM_DungeonRunPhase.COMPLETED || phase == SM_DungeonRunPhase.FAILED)
			m_RunTextHideTimer = 6.0;
		else
			m_RunTextHideTimer = 0;
	}

	protected void UpdateEvacBar()
	{
		bool active = SM_DungeonClientData.EvacActive;
		m_EvacBarBg.Show(active);
		m_EvacBarFill.Show(active);
		if (!active)
			return;

		float total = SM_DungeonClientData.EvacTotalSeconds;
		if (total <= 0)
			total = 1;
		float elapsed = total - SM_DungeonClientData.EvacSecondsRemaining;
		float ratio = elapsed / total;
		if (ratio < 0)
			ratio = 0;
		if (ratio > 1)
			ratio = 1;

		m_EvacBarFill.SetSize(360 * ratio, 18);
	}

	protected void ShowNotify(string text)
	{
		if (text == "")
			return;

		m_NotifyText.SetText(text);
		m_NotifyText.Show(true);
		m_NotifyHideTimer = 4.0;
	}
}
