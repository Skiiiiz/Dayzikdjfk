class SM_ClanMenu extends UIScriptedMenu
{
	protected const int MAP_MARKER_ROW_PERSONAL = 0;
	protected const int MAP_MARKER_ROW_CLAN = 1;
	protected const int MAP_MARKER_ROW_SERVER = 2;
	protected const int MAP_MARKER_ROW_ADMIN_BASE = 3;
	protected const int MAP_MARKER_MODE_PERSONAL = 0;
	protected const int MAP_MARKER_MODE_CLAN = 1;
	protected const int MAP_MARKER_MODE_SERVER = 2;
	protected const int MAP_MARKER_MODE_ALL = 3;
	protected const int TAB_COUNT = 16;
	protected const int BASE_RADIUS_SEGMENTS = 64;
	protected const int BASE_RADIUS_MAP_POINTS = 64;
	protected const float BASE_RADIUS_LINE_THICKNESS = 2.0;

	protected static bool s_OpenMapOnShow;
	protected static bool s_OpenMapFullscreenOnShow;

	protected TextWidget m_TitleText;
	protected ImageWidget m_TitleLogo;
	protected ButtonWidget m_CloseBtn;
	protected TextWidget m_MoneyText;
	protected TextWidget m_MoneyValueText;
	protected float m_MoneyRefreshTimer;
	protected ButtonWidget m_OnlineBtn;
	protected ImageWidget m_OnlineIcon;
	protected TextWidget m_OnlineText;
	protected TextWidget m_OnlineValueText;
	protected Widget m_PanelOnline;
	protected TextWidget m_OnlineHeaderText;
	protected TextListboxWidget m_OnlineList;
	protected ButtonWidget m_OnlineCloseBtn;
	protected float m_OnlineRefreshTimer;
	protected float m_ContractTimer;
	protected ButtonWidget m_TabMyClanBtn;
	protected ButtonWidget m_TabClansBtn;
	protected ButtonWidget m_TabInvitesBtn;
	protected ButtonWidget m_TabTopsBtn;
	protected ButtonWidget m_TabTreasuryBtn;
	protected ButtonWidget m_TabLogBtn;
	protected ButtonWidget m_TabSettingsBtn;
	protected ButtonWidget m_TabMapBtn;
	protected ButtonWidget m_TabAuctionBtn;
	protected ButtonWidget m_TabAchievementsBtn;
	protected ButtonWidget m_TabTitlesBtn;
	protected ButtonWidget m_TabContractsBtn;
	protected ButtonWidget m_TabAdminChatBtn;
	protected ref array<ButtonWidget> m_AllButtons = new array<ButtonWidget>;
	protected ref array<Widget> m_TabUnderlines = new array<Widget>;
	protected ref array<Widget> m_TabPills = new array<Widget>;
	protected ref array<ImageWidget> m_TabIcons = new array<ImageWidget>;
	protected ref array<TextWidget> m_TabLabels = new array<TextWidget>;
	protected ImageWidget m_MenuBgImage;

	protected Widget m_PanelNoClan;
	protected TextWidget m_NoClanInfoText;
	protected TextWidget m_CreateNameLabel;
	protected EditBoxWidget m_ClanNameEdit;
	protected TextWidget m_CreateNameCounterText;
	protected TextWidget m_CreateTagLabel;
	protected EditBoxWidget m_ClanTagEdit;
	protected TextWidget m_CreateTagCounterText;
	protected MultilineEditBoxWidget m_ClanDescEdit;
	protected TextWidget m_CreateDescCounterText;
	protected ImageWidget m_CreateClanLogoImage;
	protected Widget m_CreateClanBtnBg;
	protected ButtonWidget m_CreateClanBtn;
	protected TextWidget m_CreateHintText;
	protected string m_AppliedCreateLogoPaa;

	protected Widget m_PanelMyClan;
	protected TextWidget m_ClanTitleText;
	protected TextWidget m_MyRankText;
	protected TextWidget m_ClanLevelText;
	protected ImageWidget m_MembersWatermarkLogo;
	protected ImageWidget m_MembersNoiseImage;
	protected ImageWidget m_ActionsNoiseImage;
	protected TextWidget m_HdrName;
	protected TextWidget m_HdrRank;
	protected TextWidget m_HdrMemberTitle;
	protected TextWidget m_HdrStatus;
	protected TextWidget m_HdrHP;
	protected TextListboxWidget m_MembersList;
	protected ButtonWidget m_InviteBtn;
	protected ImageWidget m_InviteIcon;
	protected TextWidget m_InviteActionText;
	protected ButtonWidget m_KickBtn;
	protected ImageWidget m_KickIcon;
	protected TextWidget m_KickActionText;
	protected ButtonWidget m_PromoteBtn;
	protected ImageWidget m_PromoteIcon;
	protected TextWidget m_PromoteActionText;
	protected ButtonWidget m_DemoteBtn;
	protected ImageWidget m_DemoteIcon;
	protected TextWidget m_DemoteActionText;
	protected ButtonWidget m_LeaveBtn;
	protected ImageWidget m_LeaveIcon;
	protected TextWidget m_LeaveActionText;
	protected ButtonWidget m_DisbandBtn;
	protected ImageWidget m_DisbandIcon;
	protected TextWidget m_DisbandActionText;
	protected Widget m_DisbandConfirmOverlay;
	protected TextWidget m_DisbandConfirmTitle;
	protected TextWidget m_DisbandConfirmText;
	protected ButtonWidget m_DisbandConfirmYesBtn;
	protected ButtonWidget m_DisbandConfirmNoBtn;
	protected ButtonWidget m_DescBtn;
	protected ImageWidget m_DescIcon;
	protected TextWidget m_DescActionText;
	protected ButtonWidget m_AdminLeaveClanBtn;
	protected ImageWidget m_AdminLeaveClanIcon;
	protected TextWidget m_AdminLeaveClanActionText;
	protected TextWidget m_ColorLabel;
	protected ref array<ButtonWidget> m_ColorBtns = new array<ButtonWidget>;
	protected ref array<Widget> m_ColorSwatches = new array<Widget>;
	protected ref array<Widget> m_ColorSelected = new array<Widget>;
	protected ImageWidget m_PermsIcon;
	protected TextWidget m_PermsActionText;

	protected Widget m_PanelInvitePlayers;
	protected TextWidget m_InviteTitleText;
	protected TextListboxWidget m_PlayersList;
	protected ButtonWidget m_ConfirmInviteBtn;
	protected ButtonWidget m_RefreshPlayersBtn;
	protected ButtonWidget m_CancelInviteBtn;

	protected Widget m_PanelClans;
	protected TextWidget m_ClansHeaderText;
	protected TextWidget m_HdrClanName;
	protected TextWidget m_HdrClanTag;
	protected TextWidget m_HdrClanCount;
	protected TextWidget m_HdrClanLeader;
	protected TextWidget m_HdrClanLeaderTitle;
	protected TextListboxWidget m_ClansList;
	protected ButtonWidget m_RefreshClansBtn;

	protected Widget m_PanelInvites;
	protected TextWidget m_InvitesHeaderText;
	protected TextWidget m_HdrInvClan;
	protected TextWidget m_HdrInvFrom;
	protected TextListboxWidget m_InvitesList;
	protected ButtonWidget m_AcceptInviteBtn;
	protected ButtonWidget m_DeclineInviteBtn;

	protected Widget m_PanelTops;
	protected ref array<ButtonWidget> m_TopCatBtns = new array<ButtonWidget>;
	protected ref array<Widget> m_TopCatUnders = new array<Widget>;
	protected ref array<Widget> m_TopCatBgs = new array<Widget>;
	protected TextWidget m_TopsHeaderText;
	protected TextWidget m_TopsHdrPlace;
	protected TextWidget m_TopsHdrClan;
	protected TextWidget m_TopsHdrValue;
	protected TextListboxWidget m_TopsList;
	protected TextWidget m_TopsHintText;
	protected int m_TopCategory;
	protected float m_TopsSeasonTimer;

	protected Widget m_PanelTreasury;
	protected TextWidget m_TreasuryBalanceText;
	protected TextWidget m_TreasuryMyText;
	protected TextWidget m_TreasuryRatesText;
	protected ScrollWidget m_TreasuryRatesScroll;
	protected ButtonWidget m_DepositBtn;
	protected Widget m_DepositEditBg;
	protected EditBoxWidget m_DepositAmountEdit;
	protected ButtonWidget m_DepositAmountBtn;
	protected TextWidget m_WithdrawLabel;
	protected Widget m_WithdrawEditBg;
	protected EditBoxWidget m_WithdrawAmountEdit;
	protected ButtonWidget m_WithdrawBtn;
	protected TextWidget m_LevelInfoText;
	protected ButtonWidget m_UpgradeClanBtn;
	protected TextWidget m_TreasuryHintText;
	protected TextWidget m_TreasuryRatesHeader;
	protected TextWidget m_DepositHeader;
	protected TextWidget m_WithdrawHeader;
	protected TextWidget m_LevelHeader;
	protected TextWidget m_UpgradeProgressText;
	protected Widget m_UpgradeProgressTrack;
	protected Widget m_UpgradeProgressFill;
	protected Widget m_LevelCardBg;
	protected Widget m_LevelHeaderLine;
	protected ScrollWidget m_LevelLadderScroll;
	protected TextWidget m_LevelLadderText;
	protected Widget m_LevelColSep;

	protected Widget m_PanelLog;
	protected Widget m_PanelSettings;
	protected Widget m_SettingsRowsRoot;
	protected bool m_SettingsBuilt = false;
	protected ref array<ButtonWidget> m_SettingDecBtns = new array<ButtonWidget>;
	protected ref array<ButtonWidget> m_SettingIncBtns = new array<ButtonWidget>;
	protected ref array<TextWidget> m_SettingValueTexts = new array<TextWidget>;
	protected ref array<ButtonWidget> m_SettingToggles = new array<ButtonWidget>;
	protected ref array<ImageWidget> m_SettingToggleIcons = new array<ImageWidget>;
	protected const int SETTINGS_COUNT = 21;
	protected const int HUD_PANEL_SAFE_WIDTH = 210;
	protected const int HUD_PANEL_SAFE_COMPACT_WIDTH = 176;
	protected const int HUD_PANEL_SAFE_ROW_HEIGHT = 42;
	protected const int HUD_PANEL_SAFE_COMPACT_ROW_HEIGHT = 26;
	protected ref array<Widget> m_MemberRowWidgets = new array<Widget>;
	protected ref array<ButtonWidget> m_MemberRowDec = new array<ButtonWidget>;
	protected ref array<ButtonWidget> m_MemberRowInc = new array<ButtonWidget>;
	protected ref array<TextWidget> m_MemberRowName = new array<TextWidget>;
	protected ref array<TextWidget> m_MemberRowValue = new array<TextWidget>;
	protected ref array<string> m_HudSelUids = new array<string>;
	protected const int MAX_MEMBER_ROWS = 24;
	protected Widget m_PanelMembers;
	protected Widget m_MembersRowsRoot;
	protected ButtonWidget m_MembersCloseBtn;
	protected bool m_MembersBuilt = false;
	protected TextWidget m_LogHeaderText;
	protected TextListboxWidget m_LogList;
	protected Widget m_PanelAdminChat;
	protected TextWidget m_AdminChatHeaderText;
	protected TextWidget m_AdminChatHintText;
	protected TextListboxWidget m_AdminChatList;
	protected ButtonWidget m_AdminChatRefreshBtn;
	protected Widget m_AdminChatMutePanel;
	protected TextWidget m_AdminChatMuteTitleText;
	protected TextWidget m_AdminChatMuteTargetText;
	protected TextWidget m_AdminChatMuteMinutesLabel;
	protected Widget m_AdminChatMuteMinutesBg;
	protected EditBoxWidget m_AdminChatMuteMinutesEdit;
	protected ButtonWidget m_AdminChatMuteApplyBtn;
	protected ButtonWidget m_AdminChatUnmuteBtn;
	protected ButtonWidget m_AdminChatMuteCloseBtn;
	protected string m_AdminChatMuteUid;
	protected string m_AdminChatMuteName;
	protected ref array<string> m_AdminChatRowUids = new array<string>;
	protected ref array<string> m_AdminChatRowNames = new array<string>;

	protected Widget m_PanelMap;
	protected Widget m_MapHolderFull;
	protected Widget m_ActiveMapHolder;
	protected Widget m_MapMarkersBg;
	protected MapWidget m_ClanMap;
	protected MapWidget m_ClanMapFull;
	protected TextWidget m_MapHeaderText;
	protected TextWidget m_MapHintText;
	protected TextWidget m_MapCoordsText;
	protected ButtonWidget m_MapCloseBtn;
	protected TextListboxWidget m_MapMarkersList;
	protected TextWidget m_MapMarkerNameLabel;
	protected Widget m_MapMarkerNameBg;
	protected EditBoxWidget m_MapMarkerNameEdit;
	protected ButtonWidget m_MapModePrevBtn;
	protected ButtonWidget m_MapModeNextBtn;
	protected TextWidget m_MapModeLabel;
	protected ButtonWidget m_MapAddMarkerBtn;
	protected ButtonWidget m_MapRemoveMarkerBtn;
	protected TextWidget m_MapSelectedText;
	protected TextWidget m_MapColorLabel;
	protected ButtonWidget m_MapColorBtn;
	protected Widget m_MapColorSwatch;
	protected ButtonWidget m_MapShowMeBtn;
	protected ButtonWidget m_MapToggle3DBtn;
	protected ButtonWidget m_MapAdminBasesBtn;
	protected bool m_MapFullscreen;
	protected float m_SidebarOrigX;
	protected float m_SidebarOrigY;
	protected bool m_SidebarOrigCaptured;
	protected int m_SelectedMarkerColor;
	protected int m_SelectedMarkerIcon;
	protected float m_MapRefreshTimer;
	protected vector m_SelectedMapPosition;
	protected bool m_HasSelectedMapPosition;
	protected int m_MapMarkerMode;
	protected bool m_ShowAdminBaseMarkers;
	protected ref array<int> m_MapMarkerRowIds = new array<int>;
	protected ref array<int> m_MapMarkerRowTypes = new array<int>;
	protected Widget m_MapMarkerWidgetLayer;
	protected Widget m_MapMarkerEditPanel;
	protected TextWidget m_MapMarkerEditTitleText;
	protected EditBoxWidget m_MapMarkerEditNameEdit;
	protected Widget m_MapMarkerEditColorSwatch;
	protected EditBoxWidget m_MapMarkerEditRedEdit;
	protected EditBoxWidget m_MapMarkerEditGreenEdit;
	protected EditBoxWidget m_MapMarkerEditBlueEdit;
	protected SliderWidget m_MapMarkerEditRedSlider;
	protected SliderWidget m_MapMarkerEditGreenSlider;
	protected SliderWidget m_MapMarkerEditBlueSlider;
	protected TextWidget m_MapMarkerEditRedValueText;
	protected TextWidget m_MapMarkerEditGreenValueText;
	protected TextWidget m_MapMarkerEditBlueValueText;
	protected ButtonWidget m_MapMarkerEditRedDecBtn;
	protected ButtonWidget m_MapMarkerEditRedIncBtn;
	protected ButtonWidget m_MapMarkerEditGreenDecBtn;
	protected ButtonWidget m_MapMarkerEditGreenIncBtn;
	protected ButtonWidget m_MapMarkerEditBlueDecBtn;
	protected ButtonWidget m_MapMarkerEditBlueIncBtn;
	protected TextWidget m_MapMarkerEditInfoText;
	protected ButtonWidget m_MapMarkerEditColorPrevBtn;
	protected ButtonWidget m_MapMarkerEditColorNextBtn;
	protected ButtonWidget m_MapMarkerEditIconPrevBtn;
	protected ButtonWidget m_MapMarkerEditIconNextBtn;
	protected ImageWidget m_MapMarkerEditIconPreview;
	protected TextWidget m_MapMarkerEditIconNameText;
	protected ButtonWidget m_MapMarkerEditSaveBtn;
	protected ButtonWidget m_MapMarkerEditDeleteBtn;
	protected ButtonWidget m_MapMarkerEditCloseBtn;
	protected ref array<Widget> m_MapMarkerWidgetRoots = new array<Widget>;
	protected ref array<ButtonWidget> m_MapMarkerWidgetButtons = new array<ButtonWidget>;
	protected ref array<ImageWidget> m_MapMarkerWidgetIcons = new array<ImageWidget>;
	protected ref array<Widget> m_MapMarkerWidgetAccents = new array<Widget>;
	protected ref array<int> m_MapMarkerWidgetTypes = new array<int>;
	protected ref array<int> m_MapMarkerWidgetIds = new array<int>;
	protected ref array<vector> m_MapMarkerWidgetPositions = new array<vector>;
	protected ref array<int> m_MapMarkerWidgetColors = new array<int>;
	protected ref array<int> m_MapMarkerWidgetIconIdx = new array<int>;
	protected ref array<string> m_MapMarkerWidgetNames = new array<string>;
	protected ref array<Widget> m_MapBaseRadiusSegments = new array<Widget>;
	protected int m_MapMarkerEditType;
	protected int m_MapMarkerEditId;
	protected int m_MapMarkerEditColor;
	protected int m_MapMarkerEditIcon;
	protected vector m_MapMarkerEditPosition;
	protected bool m_MapMarkerEditNew;

	protected ButtonWidget m_TabStorageBtn;
	protected Widget m_PanelStorage;
	protected TextWidget m_StorageHeaderText;
	protected TextWidget m_HdrStorageItem;
	protected TextWidget m_HdrStorageQty;
	protected TextWidget m_HdrStorageWho;
	protected TextWidget m_HdrStorageDate;
	protected TextListboxWidget m_StorageList;
	protected Widget m_StorageTilesRoot;
	protected TextWidget m_StorageInfoText;
	protected ButtonWidget m_DepositToStorageBtn;
	protected ButtonWidget m_TakeFromStorageBtn;
	protected TextWidget m_StorageHintText;
	protected ItemPreviewWidget m_StoragePreview;
	protected EntityAI m_StoragePreviewEntity;
	protected ref array<int> m_StorageRowIds = new array<int>;
	protected ref array<Widget> m_StorageTileWidgets = new array<Widget>;
	protected ref array<EntityAI> m_StorageTileEntities = new array<EntityAI>;
	protected EditBoxWidget m_StorageSearchEdit;
	protected TextWidget m_StorageSearchHint;
	protected string m_StorageFilter;

	protected ButtonWidget m_TabMarketBtn;
	protected Widget m_PanelMarket;
	protected TextWidget m_MarketHeaderText;
	protected TextWidget m_HdrMarketItem;
	protected TextWidget m_HdrMarketQty;
	protected TextWidget m_HdrMarketPrice;
	protected TextWidget m_HdrMarketClan;
	protected TextListboxWidget m_MarketList;
	protected Widget m_MarketTilesRoot;
	protected TextWidget m_MarketInfoText;
	protected ButtonWidget m_BuyLotBtn;
	protected ButtonWidget m_CancelLotBtn;
	protected TextWidget m_SellLabel;
	protected EditBoxWidget m_SellPriceEdit;
	protected Widget m_SellPriceEditBg;
	protected ButtonWidget m_SellLotBtn;
	protected TextWidget m_MarketHintText;
	protected ItemPreviewWidget m_MarketPreview;
	protected EntityAI m_MarketPreviewEntity;
	protected ref array<int> m_MarketRowIds = new array<int>;
	protected ref array<Widget> m_MarketTileWidgets = new array<Widget>;
	protected ref array<EntityAI> m_MarketTileEntities = new array<EntityAI>;
	protected EditBoxWidget m_MarketSearchEdit;
	protected TextWidget m_MarketSearchHint;
	protected string m_MarketFilter;

	protected ButtonWidget m_TabServerMarketBtn;
	protected Widget m_PanelServerMarket;
	protected TextWidget m_ServerMarketHeaderText;
	protected TextWidget m_HdrServerMarketItem;
	protected TextWidget m_HdrServerMarketQty;
	protected TextWidget m_HdrServerMarketPrice;
	protected TextWidget m_HdrServerMarketLimit;
	protected TextListboxWidget m_ServerMarketList;
	protected Widget m_ServerMarketTilesRoot;
	protected TextWidget m_ServerMarketInfoText;
	protected ButtonWidget m_BuyServerMarketBtn;
	protected TextWidget m_ServerMarketHintText;
	protected ItemPreviewWidget m_ServerMarketPreview;
	protected EntityAI m_ServerMarketPreviewEntity;
	protected ref array<int> m_ServerMarketRowIds = new array<int>;
	protected ref array<Widget> m_ServerMarketTileWidgets = new array<Widget>;
	protected ref array<EntityAI> m_ServerMarketTileEntities = new array<EntityAI>;
	protected EditBoxWidget m_ServerMarketSearchEdit;
	protected TextWidget m_ServerMarketSearchHint;
	protected string m_ServerMarketFilter;
	protected float m_ServerMarketRefreshTimer;
	protected bool m_ServerMarketFullRefreshRequested;
	protected int m_ServerMarketShownRotationId;

	protected Widget m_PanelAuction;
	protected TextWidget m_AuctionHeaderText;
	protected TextWidget m_HdrAuctionItem;
	protected TextWidget m_HdrAuctionStart;
	protected TextWidget m_HdrAuctionBid;
	protected TextWidget m_HdrAuctionTime;
	protected TextWidget m_HdrAuctionSeller;
	protected TextListboxWidget m_AuctionList;
	protected Widget m_AuctionTilesRoot;
	protected TextWidget m_AuctionInfoText;
	protected ButtonWidget m_AuctionBidBtn;
	protected ButtonWidget m_AuctionCancelBtn;
	protected TextWidget m_AuctionStartLabel;
	protected EditBoxWidget m_AuctionStartEdit;
	protected Widget m_AuctionStartEditBg;
	protected TextWidget m_AuctionStepLabel;
	protected EditBoxWidget m_AuctionStepEdit;
	protected Widget m_AuctionStepEditBg;
	protected TextWidget m_AuctionDurationLabel;
	protected EditBoxWidget m_AuctionDurationEdit;
	protected Widget m_AuctionDurationEditBg;
	protected ButtonWidget m_AuctionSellBtn;
	protected TextWidget m_AuctionHintText;
	protected ItemPreviewWidget m_AuctionPreview;
	protected EntityAI m_AuctionPreviewEntity;
	protected ref array<int> m_AuctionRowIds = new array<int>;
	protected ref array<Widget> m_AuctionTileWidgets = new array<Widget>;
	protected ref array<EntityAI> m_AuctionTileEntities = new array<EntityAI>;
	protected EditBoxWidget m_AuctionSearchEdit;
	protected TextWidget m_AuctionSearchHint;
	protected string m_AuctionFilter;
	protected float m_AuctionTimer;

	protected Widget m_PanelAchievements;
	protected TextWidget m_AchievementsHeaderText;
	protected ButtonWidget m_AchPersonalBtn;
	protected ButtonWidget m_AchClanBtn;
	protected Widget m_AchPersonalBg;
	protected Widget m_AchPersonalUnder;
	protected Widget m_AchClanBg;
	protected Widget m_AchClanUnder;
	protected TextWidget m_HdrAchName;
	protected TextWidget m_HdrAchProgress;
	protected TextWidget m_HdrAchReward;
	protected TextListboxWidget m_AchievementsList;
	protected TextWidget m_AchievementInfoText;
	protected Widget m_AchievementProgressTrack;
	protected Widget m_AchievementProgressFill;
	protected ButtonWidget m_ClaimAchievementBtn;
	protected TextWidget m_AchievementsHintText;
	protected ref array<string> m_AchievementRowIds = new array<string>;
	protected ref array<int> m_AchievementRowScopes = new array<int>;
	protected int m_AchievementScopeFilter;

	protected Widget m_PanelTitles;
	protected TextWidget m_PlayerTitlesHeaderText;
	protected TextWidget m_PlayerTitleMineText;
	protected TextWidget m_PlayerTitleProgressText;
	protected Widget m_PlayerTitleProgressTrack;
	protected Widget m_PlayerTitleProgressFill;
	protected TextWidget m_HdrPlayerTitlePlace;
	protected TextWidget m_HdrPlayerTitleName;
	protected TextWidget m_HdrPlayerTitleTitle;
	protected TextWidget m_HdrPlayerTitleXP;
	protected TextWidget m_HdrPlayerTitleClan;
	protected TextListboxWidget m_PlayerTitlesList;
	protected ButtonWidget m_PlayerTitlesRefreshBtn;
	protected TextWidget m_PlayerTitlesHintText;

	protected Widget m_PanelContracts;
	protected TextWidget m_ContractsHeaderText;
	protected TextWidget m_HdrContractType;
	protected TextWidget m_HdrContractTarget;
	protected TextWidget m_HdrContractReward;
	protected TextWidget m_HdrContractTime;
	protected TextWidget m_HdrContractStatus;
	protected TextListboxWidget m_ContractsList;
	protected TextWidget m_ContractHistoryHeaderText;
	protected TextListboxWidget m_ContractHistoryList;
	protected TextWidget m_ContractTargetsHeaderText;
	protected TextListboxWidget m_ContractTargetsList;
	protected Widget m_ContractTargetPreviewPanel;
	protected PlayerPreviewWidget m_ContractTargetPreview;
	protected TextWidget m_ContractTargetPreviewName;
	protected ButtonWidget m_ContractTargetPreviewCloseBtn;
	protected DayZPlayer m_ContractTargetPreviewPlayer;
	protected EntityAI m_ContractTargetPreviewHand;
	protected TextWidget m_ContractItemsHeaderText;
	protected TextListboxWidget m_ContractItemsList;
	protected EditBoxWidget m_ContractPriceEdit;
	protected EditBoxWidget m_ContractDurationEdit;
	protected EditBoxWidget m_ContractQuantityEdit;
	protected ButtonWidget m_ContractCreateKillBtn;
	protected ButtonWidget m_ContractCreateItemBtn;
	protected ButtonWidget m_ContractAcceptBtn;
	protected ButtonWidget m_ContractAbandonBtn;
	protected ButtonWidget m_ContractTurnInBtn;
	protected ButtonWidget m_ContractRefreshBtn;
	protected TextWidget m_ContractInfoText;
	protected ref array<int> m_ContractRowIds = new array<int>;
	protected ref array<int> m_ContractHistoryRowIds = new array<int>;
	protected ref array<string> m_ContractTargetRowUids = new array<string>;
	protected ref array<string> m_ContractItemRowClasses = new array<string>;

	protected Widget m_PanelDescEdit;
	protected TextWidget m_DescTitleText;
	protected MultilineEditBoxWidget m_DescEdit;
	protected TextWidget m_DescViewText;
	protected ButtonWidget m_SaveDescBtn;
	protected ButtonWidget m_CloseDescBtn;
	protected TextWidget m_DescHintText;

	protected ButtonWidget m_PermsBtn;
	protected Widget m_PanelPerms;
	protected TextWidget m_PermsHeaderText;
	protected TextWidget m_PermsHdrAction;
	protected TextWidget m_PermsHdrRank;
	protected TextListboxWidget m_PermsList;
	protected ButtonWidget m_PermRankUpBtn;
	protected ButtonWidget m_PermRankDownBtn;
	protected ButtonWidget m_PermsCloseBtn;
	protected TextWidget m_PermsHintText;
	protected TextWidget m_WebhookStatusText;
	protected Widget m_WebhookEditBg;
	protected EditBoxWidget m_WebhookEdit;
	protected ButtonWidget m_WebhookSaveBtn;
	protected ButtonWidget m_WebhookClearBtn;
	protected ButtonWidget m_WebhookTestBtn;
	protected ButtonWidget m_WebhookAuditBtn;
	protected bool m_PermsMode;
	protected ref array<int> m_PermRowActions = new array<int>;

	protected Widget m_PanelClanInfo;
	protected TextWidget m_InfoTitleText;
	protected TextWidget m_InfoDescLabel;
	protected TextWidget m_InfoDescText;
	protected TextWidget m_InfoMembersLabel;
	protected TextWidget m_InfoHdrName;
	protected TextWidget m_InfoHdrRank;
	protected TextWidget m_InfoHdrTitle;
	protected TextWidget m_InfoHdrStatus;
	protected TextListboxWidget m_InfoMembersList;
	protected ButtonWidget m_ApplyBtn;
	protected ButtonWidget m_AdminEnterClanBtn;
	protected ButtonWidget m_CloseInfoBtn;

	protected int m_CurrentTab;
	protected bool m_InviteMode;
	protected bool m_DescMode;
	protected bool m_InfoMode;

	protected ref array<string> m_MemberRowUids = new array<string>;
	protected ref array<string> m_PlayerRowUids = new array<string>;
	protected ref array<string> m_InviteRowClans = new array<string>;
	protected ref array<string> m_ApplicationRowUids = new array<string>;
	protected ref array<string> m_ClanRowNames = new array<string>;

	override int GetID()
	{
		return SM_PARTY_MENU_ID;
	}

	bool IsMapTabVisible()
	{
		if (m_CurrentTab == 6 && SM_ClanClientData.MapEnabled)
			return true;
		return false;
	}

	void SM_CloseFromHotkey(bool respectDisbandConfirm)
	{
		if (IsClosing())
			return;

		if (respectDisbandConfirm && IsDisbandConfirmOpen())
		{
			HideDisbandConfirm();
			return;
		}

		Close();
	}

	bool SM_IsTextInputFocused()
	{
		Widget focus = GetFocus();
		if (!focus)
			return false;

		if (EditBoxWidget.Cast(focus))
			return true;

		if (MultilineEditBoxWidget.Cast(focus))
			return true;

		return false;
	}

	protected void SuppressMouseLookInput()
	{
		UAInputAPI api = GetUApi();
		if (!api)
			return;

		UAInput personView = api.GetInputByID(UAPersonView);
		if (personView)
			personView.Supress();
	}

	protected void FocusMenuInput()
	{
		if (m_CurrentTab == 6 && SM_ClanClientData.MapEnabled && m_ClanMap)
		{
			SetFocus(m_ClanMap);
			return;
		}

		if (layoutRoot)
			SetFocus(layoutRoot);
	}

	// Открыть меню сразу на вкладке карты и развернуть её на весь экран.
	static void RequestOpenMapFullscreenOnShow()
	{
		s_OpenMapOnShow = true;
		s_OpenMapFullscreenOnShow = true;
	}

	override bool UseMouse()
	{
		return true;
	}

	override bool UseKeyboard()
	{
		return true;
	}

	override Widget Init()
	{
		layoutRoot = GetGame().GetWorkspace().CreateWidgets("SM_PartyMod/GUI/layouts/SM_ClanMenu.layout");

		m_MenuBgImage = ImageWidget.Cast(layoutRoot.FindAnyWidget("MenuBgImage"));
		m_TitleText = TextWidget.Cast(layoutRoot.FindAnyWidget("TitleText"));
		m_TitleLogo = ImageWidget.Cast(layoutRoot.FindAnyWidget("TitleLogo"));
		m_CloseBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("CloseBtn"));
		m_MoneyText = TextWidget.Cast(layoutRoot.FindAnyWidget("MoneyText"));
		m_MoneyValueText = TextWidget.Cast(layoutRoot.FindAnyWidget("MoneyValueText"));
		m_OnlineBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("OnlineBtn"));
		m_OnlineIcon = ImageWidget.Cast(layoutRoot.FindAnyWidget("OnlineIcon"));
		m_OnlineText = TextWidget.Cast(layoutRoot.FindAnyWidget("OnlineText"));
		m_OnlineValueText = TextWidget.Cast(layoutRoot.FindAnyWidget("OnlineValueText"));
		m_PanelOnline = layoutRoot.FindAnyWidget("PanelOnline");
		m_OnlineHeaderText = TextWidget.Cast(layoutRoot.FindAnyWidget("OnlineHeaderText"));
		m_OnlineList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("OnlineList"));
		m_OnlineCloseBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("OnlineCloseBtn"));
		m_TabMyClanBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("TabMyClanBtn"));
		m_TabClansBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("TabClansBtn"));
		m_TabInvitesBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("TabInvitesBtn"));
		m_TabTopsBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("TabTopsBtn"));
		m_TabTreasuryBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("TabTreasuryBtn"));
		m_TabLogBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("TabLogBtn"));
		m_TabSettingsBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("TabSettingsBtn"));
		m_TabMapBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("TabMapBtn"));
		m_TabServerMarketBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("TabServerMarketBtn"));
		m_TabStorageBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("TabStorageBtn"));
		m_TabAuctionBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("TabAuctionBtn"));
		m_TabAchievementsBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("TabAchievementsBtn"));
		m_TabTitlesBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("TabTitlesBtn"));
		m_TabContractsBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("TabContractsBtn"));
		m_TabAdminChatBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("TabAdminChatBtn"));

		m_TabUnderlines.Clear();
		for (int u = 0; u < TAB_COUNT; u++)
			m_TabUnderlines.Insert(layoutRoot.FindAnyWidget("TabUnderline" + u.ToString()));

		m_TabPills.Clear();
		for (int p = 0; p < TAB_COUNT; p++)
			m_TabPills.Insert(layoutRoot.FindAnyWidget("TabPill" + p.ToString()));

		m_TabIcons.Clear();
		m_TabLabels.Clear();
		for (int tab = 0; tab < TAB_COUNT; tab++)
		{
			m_TabIcons.Insert(ImageWidget.Cast(layoutRoot.FindAnyWidget("TabIcon" + tab.ToString())));
			m_TabLabels.Insert(TextWidget.Cast(layoutRoot.FindAnyWidget("TabLabel" + tab.ToString())));
		}

		m_PanelNoClan = layoutRoot.FindAnyWidget("PanelNoClan");
		m_NoClanInfoText = TextWidget.Cast(layoutRoot.FindAnyWidget("NoClanInfoText"));
		m_CreateNameLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("CreateNameLabel"));
		m_ClanNameEdit = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("ClanNameEdit"));
		m_CreateNameCounterText = TextWidget.Cast(layoutRoot.FindAnyWidget("CreateNameCounterText"));
		m_CreateTagLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("CreateTagLabel"));
		m_ClanTagEdit = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("ClanTagEdit"));
		m_CreateTagCounterText = TextWidget.Cast(layoutRoot.FindAnyWidget("CreateTagCounterText"));
		m_ClanDescEdit = MultilineEditBoxWidget.Cast(layoutRoot.FindAnyWidget("ClanDescEdit"));
		m_CreateDescCounterText = TextWidget.Cast(layoutRoot.FindAnyWidget("CreateDescCounterText"));
		m_CreateClanLogoImage = ImageWidget.Cast(layoutRoot.FindAnyWidget("CreateClanLogoImage"));
		m_CreateClanBtnBg = layoutRoot.FindAnyWidget("CreateClanBtnBg");
		m_CreateClanBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("CreateClanBtn"));
		m_CreateHintText = TextWidget.Cast(layoutRoot.FindAnyWidget("CreateHintText"));

		m_PanelMyClan = layoutRoot.FindAnyWidget("PanelMyClan");
		m_ClanTitleText = TextWidget.Cast(layoutRoot.FindAnyWidget("ClanTitleText"));
		m_MyRankText = TextWidget.Cast(layoutRoot.FindAnyWidget("MyRankText"));
		m_ClanLevelText = TextWidget.Cast(layoutRoot.FindAnyWidget("ClanLevelText"));
		m_MembersWatermarkLogo = ImageWidget.Cast(layoutRoot.FindAnyWidget("MembersWatermarkLogo"));
		m_MembersNoiseImage = ImageWidget.Cast(layoutRoot.FindAnyWidget("MembersNoiseImage"));
		m_ActionsNoiseImage = ImageWidget.Cast(layoutRoot.FindAnyWidget("ActionsNoiseImage"));
		m_HdrName = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrName"));
		m_HdrRank = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrRank"));
		m_HdrMemberTitle = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrMemberTitle"));
		m_HdrStatus = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrStatus"));
		m_HdrHP = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrHP"));
		m_MembersList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("MembersList"));
		m_InviteBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("InviteBtn"));
		m_InviteIcon = ImageWidget.Cast(layoutRoot.FindAnyWidget("InviteIcon"));
		m_InviteActionText = TextWidget.Cast(layoutRoot.FindAnyWidget("InviteActionText"));
		m_KickBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("KickBtn"));
		m_KickIcon = ImageWidget.Cast(layoutRoot.FindAnyWidget("KickIcon"));
		m_KickActionText = TextWidget.Cast(layoutRoot.FindAnyWidget("KickActionText"));
		m_PromoteBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("PromoteBtn"));
		m_PromoteIcon = ImageWidget.Cast(layoutRoot.FindAnyWidget("PromoteIcon"));
		m_PromoteActionText = TextWidget.Cast(layoutRoot.FindAnyWidget("PromoteActionText"));
		m_DemoteBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("DemoteBtn"));
		m_DemoteIcon = ImageWidget.Cast(layoutRoot.FindAnyWidget("DemoteIcon"));
		m_DemoteActionText = TextWidget.Cast(layoutRoot.FindAnyWidget("DemoteActionText"));
		m_LeaveBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("LeaveBtn"));
		m_LeaveIcon = ImageWidget.Cast(layoutRoot.FindAnyWidget("LeaveIcon"));
		m_LeaveActionText = TextWidget.Cast(layoutRoot.FindAnyWidget("LeaveActionText"));
		m_DisbandBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("DisbandBtn"));
		m_DisbandIcon = ImageWidget.Cast(layoutRoot.FindAnyWidget("DisbandIcon"));
		m_DisbandActionText = TextWidget.Cast(layoutRoot.FindAnyWidget("DisbandActionText"));
		m_DisbandConfirmOverlay = layoutRoot.FindAnyWidget("DisbandConfirmOverlay");
		m_DisbandConfirmTitle = TextWidget.Cast(layoutRoot.FindAnyWidget("DisbandConfirmTitle"));
		m_DisbandConfirmText = TextWidget.Cast(layoutRoot.FindAnyWidget("DisbandConfirmText"));
		m_DisbandConfirmYesBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("DisbandConfirmYesBtn"));
		m_DisbandConfirmNoBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("DisbandConfirmNoBtn"));
		m_DescBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("DescBtn"));
		m_DescIcon = ImageWidget.Cast(layoutRoot.FindAnyWidget("DescIcon"));
		m_DescActionText = TextWidget.Cast(layoutRoot.FindAnyWidget("DescActionText"));
		m_PermsBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("PermsBtn"));
		m_PermsIcon = ImageWidget.Cast(layoutRoot.FindAnyWidget("PermsIcon"));
		m_PermsActionText = TextWidget.Cast(layoutRoot.FindAnyWidget("PermsActionText"));
		m_AdminLeaveClanBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("AdminLeaveClanBtn"));
		m_AdminLeaveClanIcon = ImageWidget.Cast(layoutRoot.FindAnyWidget("AdminLeaveClanIcon"));
		m_AdminLeaveClanActionText = TextWidget.Cast(layoutRoot.FindAnyWidget("AdminLeaveClanActionText"));
		m_ColorLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("ColorLabel"));

		m_ColorBtns.Clear();
		m_ColorSwatches.Clear();
		m_ColorSelected.Clear();
		for (int c = 0; c < SM_ClanColors.Count(); c++)
		{
			ButtonWidget colorBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("ColorBtn" + c.ToString()));
			Widget swatch = layoutRoot.FindAnyWidget("ColorSwatch" + c.ToString());
			Widget selected = layoutRoot.FindAnyWidget("ColorSelected" + c.ToString());
			if (swatch)
				swatch.SetColor(SM_ClanColors.GetColor(c));
			m_ColorBtns.Insert(colorBtn);
			m_ColorSwatches.Insert(swatch);
			m_ColorSelected.Insert(selected);
		}

		m_PanelInvitePlayers = layoutRoot.FindAnyWidget("PanelInvitePlayers");
		m_InviteTitleText = TextWidget.Cast(layoutRoot.FindAnyWidget("InviteTitleText"));
		m_PlayersList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("PlayersList"));
		m_ConfirmInviteBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("ConfirmInviteBtn"));
		m_RefreshPlayersBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("RefreshPlayersBtn"));
		m_CancelInviteBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("CancelInviteBtn"));

		m_PanelClans = layoutRoot.FindAnyWidget("PanelClans");
		m_ClansHeaderText = TextWidget.Cast(layoutRoot.FindAnyWidget("ClansHeaderText"));
		m_HdrClanName = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrClanName"));
		m_HdrClanTag = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrClanTag"));
		m_HdrClanCount = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrClanCount"));
		m_HdrClanLeader = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrClanLeader"));
		m_HdrClanLeaderTitle = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrClanLeaderTitle"));
		m_ClansList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("ClansList"));
		m_RefreshClansBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("RefreshClansBtn"));

		m_PanelInvites = layoutRoot.FindAnyWidget("PanelInvites");
		m_InvitesHeaderText = TextWidget.Cast(layoutRoot.FindAnyWidget("InvitesHeaderText"));
		m_HdrInvClan = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrInvClan"));
		m_HdrInvFrom = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrInvFrom"));
		m_InvitesList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("InvitesList"));
		m_AcceptInviteBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("AcceptInviteBtn"));
		m_DeclineInviteBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("DeclineInviteBtn"));

		m_PanelTops = layoutRoot.FindAnyWidget("PanelTops");
		m_TopCatBtns.Clear();
		m_TopCatUnders.Clear();
		m_TopCatBgs.Clear();
		for (int t = 0; t < SM_TopCategory.COUNT; t++)
		{
			m_TopCatBtns.Insert(ButtonWidget.Cast(layoutRoot.FindAnyWidget("TopCatBtn" + t.ToString())));
			m_TopCatUnders.Insert(layoutRoot.FindAnyWidget("TopCatUnder" + t.ToString()));
			m_TopCatBgs.Insert(layoutRoot.FindAnyWidget("TopCatBg" + t.ToString()));
		}
		m_TopsHeaderText = TextWidget.Cast(layoutRoot.FindAnyWidget("TopsHeaderText"));
		m_TopsHdrPlace = TextWidget.Cast(layoutRoot.FindAnyWidget("TopsHdrPlace"));
		m_TopsHdrClan = TextWidget.Cast(layoutRoot.FindAnyWidget("TopsHdrClan"));
		m_TopsHdrValue = TextWidget.Cast(layoutRoot.FindAnyWidget("TopsHdrValue"));
		m_TopsList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("TopsList"));
		m_TopsHintText = TextWidget.Cast(layoutRoot.FindAnyWidget("TopsHintText"));

		m_PanelTreasury = layoutRoot.FindAnyWidget("PanelTreasury");
		m_TreasuryBalanceText = TextWidget.Cast(layoutRoot.FindAnyWidget("TreasuryBalanceText"));
		m_TreasuryMyText = TextWidget.Cast(layoutRoot.FindAnyWidget("TreasuryMyText"));
		m_TreasuryRatesText = TextWidget.Cast(layoutRoot.FindAnyWidget("TreasuryRatesText"));
		m_TreasuryRatesScroll = ScrollWidget.Cast(layoutRoot.FindAnyWidget("TreasuryRatesScroll"));
		m_DepositBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("DepositBtn"));
		m_DepositEditBg = layoutRoot.FindAnyWidget("DepositEditBg");
		m_DepositAmountEdit = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("DepositAmountEdit"));
		m_DepositAmountBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("DepositAmountBtn"));
		m_WithdrawLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("WithdrawLabel"));
		m_WithdrawEditBg = layoutRoot.FindAnyWidget("WithdrawEditBg");
		m_WithdrawAmountEdit = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("WithdrawAmountEdit"));
		m_WithdrawBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("WithdrawBtn"));
		m_LevelInfoText = TextWidget.Cast(layoutRoot.FindAnyWidget("LevelInfoText"));
		m_UpgradeClanBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("UpgradeClanBtn"));
		m_TreasuryHintText = TextWidget.Cast(layoutRoot.FindAnyWidget("TreasuryHintText"));
		m_TreasuryRatesHeader = TextWidget.Cast(layoutRoot.FindAnyWidget("TreasuryRatesHeader"));
		m_DepositHeader = TextWidget.Cast(layoutRoot.FindAnyWidget("DepositHeader"));
		m_WithdrawHeader = TextWidget.Cast(layoutRoot.FindAnyWidget("WithdrawHeader"));
		m_LevelHeader = TextWidget.Cast(layoutRoot.FindAnyWidget("LevelHeader"));
		m_UpgradeProgressText = TextWidget.Cast(layoutRoot.FindAnyWidget("UpgradeProgressText"));
		m_UpgradeProgressTrack = layoutRoot.FindAnyWidget("UpgradeProgressTrack");
		m_UpgradeProgressFill = layoutRoot.FindAnyWidget("UpgradeProgressFill");
		m_LevelCardBg = layoutRoot.FindAnyWidget("LevelCardBg");
		m_LevelHeaderLine = layoutRoot.FindAnyWidget("LevelHeaderLine");
		m_LevelLadderScroll = ScrollWidget.Cast(layoutRoot.FindAnyWidget("LevelLadderScroll"));
		m_LevelLadderText = TextWidget.Cast(layoutRoot.FindAnyWidget("LevelLadderText"));
		m_LevelColSep = layoutRoot.FindAnyWidget("LevelColSep");

		m_PanelLog = layoutRoot.FindAnyWidget("PanelLog");
		m_PanelSettings = layoutRoot.FindAnyWidget("PanelSettings");
		m_SettingsRowsRoot = layoutRoot.FindAnyWidget("SettingsRowsRoot");
		m_PanelMembers = layoutRoot.FindAnyWidget("PanelMembers");
		m_MembersRowsRoot = layoutRoot.FindAnyWidget("MembersRowsRoot");
		m_MembersCloseBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("MembersCloseBtn"));
		m_LogHeaderText = TextWidget.Cast(layoutRoot.FindAnyWidget("LogHeaderText"));
		m_LogList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("LogList"));
		m_PanelAdminChat = layoutRoot.FindAnyWidget("PanelAdminChat");
		m_AdminChatHeaderText = TextWidget.Cast(layoutRoot.FindAnyWidget("AdminChatHeaderText"));
		m_AdminChatHintText = TextWidget.Cast(layoutRoot.FindAnyWidget("AdminChatHintText"));
		m_AdminChatList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("AdminChatList"));
		m_AdminChatRefreshBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("AdminChatRefreshBtn"));
		m_AdminChatMutePanel = layoutRoot.FindAnyWidget("AdminChatMutePanel");
		m_AdminChatMuteTitleText = TextWidget.Cast(layoutRoot.FindAnyWidget("AdminChatMuteTitleText"));
		m_AdminChatMuteTargetText = TextWidget.Cast(layoutRoot.FindAnyWidget("AdminChatMuteTargetText"));
		m_AdminChatMuteMinutesLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("AdminChatMuteMinutesLabel"));
		m_AdminChatMuteMinutesBg = layoutRoot.FindAnyWidget("AdminChatMuteMinutesBg");
		m_AdminChatMuteMinutesEdit = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("AdminChatMuteMinutesEdit"));
		m_AdminChatMuteApplyBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("AdminChatMuteApplyBtn"));
		m_AdminChatUnmuteBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("AdminChatUnmuteBtn"));
		m_AdminChatMuteCloseBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("AdminChatMuteCloseBtn"));

		m_PanelMap = layoutRoot.FindAnyWidget("PanelMap");
		m_MapHolderFull = layoutRoot.FindAnyWidget("MapPanelHolderFull");
		m_MapMarkersBg = layoutRoot.FindAnyWidget("MapMarkersBg");
		m_ClanMapFull = MapWidget.Cast(layoutRoot.FindAnyWidget("ClanMapWidgetFull"));
		m_ActiveMapHolder = m_MapHolderFull;
		m_ClanMap = m_ClanMapFull;
		m_MapHeaderText = TextWidget.Cast(layoutRoot.FindAnyWidget("MapHeaderText"));
		m_MapHintText = TextWidget.Cast(layoutRoot.FindAnyWidget("MapHintText"));
		m_MapCoordsText = TextWidget.Cast(layoutRoot.FindAnyWidget("MapCoordsText"));
		if (m_MapCoordsText)
		{
			m_MapCoordsText.SetOutline(2, ARGB(255, 0, 0, 0));
			m_MapCoordsText.SetShadow(3, ARGB(255, 0, 0, 0), 1.0, 1, 1);
		}
		m_MapCloseBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("MapCloseBtn"));
		m_MapMarkersList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("MapMarkersList"));
		m_MapMarkerNameLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerNameLabel"));
		m_MapMarkerNameBg = layoutRoot.FindAnyWidget("MapMarkerNameBg");
		m_MapMarkerNameEdit = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerNameEdit"));
		m_MapModePrevBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("MapModePrevBtn"));
		m_MapModeNextBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("MapModeNextBtn"));
		m_MapModeLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("MapModeLabel"));
		m_MapAddMarkerBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("MapAddMarkerBtn"));
		m_MapRemoveMarkerBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("MapRemoveMarkerBtn"));
		m_MapSelectedText = TextWidget.Cast(layoutRoot.FindAnyWidget("MapSelectedText"));
		m_MapColorLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("MapColorLabel"));
		m_MapColorBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("MapColorBtn"));
		m_MapColorSwatch = layoutRoot.FindAnyWidget("MapColorSwatch");
		m_MapShowMeBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("MapShowMeBtn"));
		m_MapToggle3DBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("MapToggle3DBtn"));
		m_MapAdminBasesBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("MapAdminBasesBtn"));
		m_MapMarkerWidgetLayer = layoutRoot.FindAnyWidget("MapMarkerWidgetLayer");
		m_MapMarkerEditPanel = layoutRoot.FindAnyWidget("MapMarkerEditPanel");
		m_MapMarkerEditTitleText = TextWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditTitleText"));
		m_MapMarkerEditNameEdit = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditNameEdit"));
		m_MapMarkerEditColorSwatch = layoutRoot.FindAnyWidget("MapMarkerEditColorSwatch");
		m_MapMarkerEditRedEdit = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditRedEdit"));
		m_MapMarkerEditGreenEdit = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditGreenEdit"));
		m_MapMarkerEditBlueEdit = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditBlueEdit"));
		m_MapMarkerEditRedSlider = SliderWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditRedSlider"));
		m_MapMarkerEditGreenSlider = SliderWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditGreenSlider"));
		m_MapMarkerEditBlueSlider = SliderWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditBlueSlider"));
		m_MapMarkerEditRedValueText = TextWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditRedValueText"));
		m_MapMarkerEditGreenValueText = TextWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditGreenValueText"));
		m_MapMarkerEditBlueValueText = TextWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditBlueValueText"));
		m_MapMarkerEditRedDecBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditRedDecBtn"));
		m_MapMarkerEditRedIncBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditRedIncBtn"));
		m_MapMarkerEditGreenDecBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditGreenDecBtn"));
		m_MapMarkerEditGreenIncBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditGreenIncBtn"));
		m_MapMarkerEditBlueDecBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditBlueDecBtn"));
		m_MapMarkerEditBlueIncBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditBlueIncBtn"));
		m_MapMarkerEditInfoText = TextWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditInfoText"));
		m_MapMarkerEditColorPrevBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditColorPrevBtn"));
		m_MapMarkerEditColorNextBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditColorNextBtn"));
		m_MapMarkerEditIconPrevBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditIconPrevBtn"));
		m_MapMarkerEditIconNextBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditIconNextBtn"));
		m_MapMarkerEditIconPreview = ImageWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditIconPreview"));
		m_MapMarkerEditIconNameText = TextWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditIconNameText"));
		m_MapMarkerEditSaveBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditSaveBtn"));
		m_MapMarkerEditDeleteBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditDeleteBtn"));
		m_MapMarkerEditCloseBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("MapMarkerEditCloseBtn"));
		if (m_MapCloseBtn)
			m_MapCloseBtn.Show(false);

		m_TabMarketBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("TabMarketBtn"));
		m_PanelMarket = layoutRoot.FindAnyWidget("PanelMarket");
		m_MarketHeaderText = TextWidget.Cast(layoutRoot.FindAnyWidget("MarketHeaderText"));
		m_HdrMarketItem = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrMarketItem"));
		m_HdrMarketQty = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrMarketQty"));
		m_HdrMarketPrice = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrMarketPrice"));
		m_HdrMarketClan = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrMarketClan"));
		m_MarketList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("MarketList"));
		m_MarketTilesRoot = layoutRoot.FindAnyWidget("MarketTilesRoot");
		m_MarketInfoText = TextWidget.Cast(layoutRoot.FindAnyWidget("MarketInfoText"));
		m_BuyLotBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BuyLotBtn"));
		m_CancelLotBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("CancelLotBtn"));
		m_SellLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("SellLabel"));
		m_SellPriceEdit = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("SellPriceEdit"));
		m_SellPriceEditBg = layoutRoot.FindAnyWidget("SellPriceEditBg");
		m_SellLotBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SellLotBtn"));
		m_MarketHintText = TextWidget.Cast(layoutRoot.FindAnyWidget("MarketHintText"));
		m_MarketPreview = ItemPreviewWidget.Cast(layoutRoot.FindAnyWidget("MarketPreview"));
		m_MarketSearchEdit = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("MarketSearchEdit"));
		m_MarketSearchHint = TextWidget.Cast(layoutRoot.FindAnyWidget("MarketSearchHint"));

		m_PanelServerMarket = layoutRoot.FindAnyWidget("PanelServerMarket");
		m_ServerMarketHeaderText = TextWidget.Cast(layoutRoot.FindAnyWidget("ServerMarketHeaderText"));
		m_HdrServerMarketItem = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrServerMarketItem"));
		m_HdrServerMarketQty = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrServerMarketQty"));
		m_HdrServerMarketPrice = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrServerMarketPrice"));
		m_HdrServerMarketLimit = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrServerMarketLimit"));
		m_ServerMarketList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("ServerMarketList"));
		m_ServerMarketTilesRoot = layoutRoot.FindAnyWidget("ServerMarketTilesRoot");
		m_ServerMarketInfoText = TextWidget.Cast(layoutRoot.FindAnyWidget("ServerMarketInfoText"));
		m_BuyServerMarketBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("BuyServerMarketBtn"));
		m_ServerMarketHintText = TextWidget.Cast(layoutRoot.FindAnyWidget("ServerMarketHintText"));
		m_ServerMarketPreview = ItemPreviewWidget.Cast(layoutRoot.FindAnyWidget("ServerMarketPreview"));
		m_ServerMarketSearchEdit = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("ServerMarketSearchEdit"));
		m_ServerMarketSearchHint = TextWidget.Cast(layoutRoot.FindAnyWidget("ServerMarketSearchHint"));

		m_PanelAuction = layoutRoot.FindAnyWidget("PanelAuction");
		m_AuctionHeaderText = TextWidget.Cast(layoutRoot.FindAnyWidget("AuctionHeaderText"));
		m_HdrAuctionItem = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrAuctionItem"));
		m_HdrAuctionStart = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrAuctionStart"));
		m_HdrAuctionBid = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrAuctionBid"));
		m_HdrAuctionTime = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrAuctionTime"));
		m_HdrAuctionSeller = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrAuctionSeller"));
		m_AuctionList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("AuctionList"));
		m_AuctionTilesRoot = layoutRoot.FindAnyWidget("AuctionTilesRoot");
		m_AuctionInfoText = TextWidget.Cast(layoutRoot.FindAnyWidget("AuctionInfoText"));
		m_AuctionBidBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("AuctionBidBtn"));
		m_AuctionCancelBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("AuctionCancelBtn"));
		m_AuctionStartLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("AuctionStartLabel"));
		m_AuctionStartEdit = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("AuctionStartEdit"));
		m_AuctionStartEditBg = layoutRoot.FindAnyWidget("AuctionStartEditBg");
		m_AuctionStepLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("AuctionStepLabel"));
		m_AuctionStepEdit = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("AuctionStepEdit"));
		m_AuctionStepEditBg = layoutRoot.FindAnyWidget("AuctionStepEditBg");
		m_AuctionDurationLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("AuctionDurationLabel"));
		m_AuctionDurationEdit = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("AuctionDurationEdit"));
		m_AuctionDurationEditBg = layoutRoot.FindAnyWidget("AuctionDurationEditBg");
		m_AuctionSellBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("AuctionSellBtn"));
		m_AuctionHintText = TextWidget.Cast(layoutRoot.FindAnyWidget("AuctionHintText"));
		m_AuctionPreview = ItemPreviewWidget.Cast(layoutRoot.FindAnyWidget("AuctionPreview"));
		m_AuctionSearchEdit = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("AuctionSearchEdit"));
		m_AuctionSearchHint = TextWidget.Cast(layoutRoot.FindAnyWidget("AuctionSearchHint"));

		m_PanelAchievements = layoutRoot.FindAnyWidget("PanelAchievements");
		m_AchievementsHeaderText = TextWidget.Cast(layoutRoot.FindAnyWidget("AchievementsHeaderText"));
		m_AchPersonalBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("AchPersonalBtn"));
		m_AchClanBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("AchClanBtn"));
		m_AchPersonalBg = layoutRoot.FindAnyWidget("AchPersonalBg");
		m_AchPersonalUnder = layoutRoot.FindAnyWidget("AchPersonalUnder");
		m_AchClanBg = layoutRoot.FindAnyWidget("AchClanBg");
		m_AchClanUnder = layoutRoot.FindAnyWidget("AchClanUnder");
		m_HdrAchName = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrAchName"));
		m_HdrAchProgress = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrAchProgress"));
		m_HdrAchReward = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrAchReward"));
		m_AchievementsList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("AchievementsList"));
		m_AchievementInfoText = TextWidget.Cast(layoutRoot.FindAnyWidget("AchievementInfoText"));
		m_AchievementProgressTrack = layoutRoot.FindAnyWidget("AchievementProgressTrack");
		m_AchievementProgressFill = layoutRoot.FindAnyWidget("AchievementProgressFill");
		m_ClaimAchievementBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("ClaimAchievementBtn"));
		m_AchievementsHintText = TextWidget.Cast(layoutRoot.FindAnyWidget("AchievementsHintText"));

		m_PanelTitles = layoutRoot.FindAnyWidget("PanelTitles");
		m_PlayerTitlesHeaderText = TextWidget.Cast(layoutRoot.FindAnyWidget("PlayerTitlesHeaderText"));
		m_PlayerTitleMineText = TextWidget.Cast(layoutRoot.FindAnyWidget("PlayerTitleMineText"));
		m_PlayerTitleProgressText = TextWidget.Cast(layoutRoot.FindAnyWidget("PlayerTitleProgressText"));
		m_PlayerTitleProgressTrack = layoutRoot.FindAnyWidget("PlayerTitleProgressTrack");
		m_PlayerTitleProgressFill = layoutRoot.FindAnyWidget("PlayerTitleProgressFill");
		m_HdrPlayerTitlePlace = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrPlayerTitlePlace"));
		m_HdrPlayerTitleName = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrPlayerTitleName"));
		m_HdrPlayerTitleTitle = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrPlayerTitleTitle"));
		m_HdrPlayerTitleXP = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrPlayerTitleXP"));
		m_HdrPlayerTitleClan = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrPlayerTitleClan"));
		m_PlayerTitlesList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("PlayerTitlesList"));
		m_PlayerTitlesRefreshBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("PlayerTitlesRefreshBtn"));
		m_PlayerTitlesHintText = TextWidget.Cast(layoutRoot.FindAnyWidget("PlayerTitlesHintText"));

		m_PanelContracts = layoutRoot.FindAnyWidget("PanelContracts");
		m_ContractsHeaderText = TextWidget.Cast(layoutRoot.FindAnyWidget("ContractsHeaderText"));
		m_HdrContractType = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrContractType"));
		m_HdrContractTarget = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrContractTarget"));
		m_HdrContractReward = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrContractReward"));
		m_HdrContractTime = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrContractTime"));
		m_HdrContractStatus = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrContractStatus"));
		m_ContractsList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("ContractsList"));
		m_ContractHistoryHeaderText = TextWidget.Cast(layoutRoot.FindAnyWidget("ContractHistoryHeaderText"));
		m_ContractHistoryList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("ContractHistoryList"));
		m_ContractTargetsHeaderText = TextWidget.Cast(layoutRoot.FindAnyWidget("ContractTargetsHeaderText"));
		m_ContractTargetsList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("ContractTargetsList"));
		m_ContractTargetPreviewPanel = layoutRoot.FindAnyWidget("ContractTargetPreviewPanel");
		m_ContractTargetPreview = PlayerPreviewWidget.Cast(layoutRoot.FindAnyWidget("ContractTargetPreview"));
		m_ContractTargetPreviewName = TextWidget.Cast(layoutRoot.FindAnyWidget("ContractTargetPreviewName"));
		m_ContractTargetPreviewCloseBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("ContractTargetPreviewCloseBtn"));
		m_ContractItemsHeaderText = TextWidget.Cast(layoutRoot.FindAnyWidget("ContractItemsHeaderText"));
		m_ContractItemsList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("ContractItemsList"));
		m_ContractPriceEdit = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("ContractPriceEdit"));
		m_ContractDurationEdit = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("ContractDurationEdit"));
		m_ContractQuantityEdit = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("ContractQuantityEdit"));
		m_ContractCreateKillBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("ContractCreateKillBtn"));
		m_ContractCreateItemBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("ContractCreateItemBtn"));
		m_ContractAcceptBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("ContractAcceptBtn"));
		m_ContractAbandonBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("ContractAbandonBtn"));
		m_ContractTurnInBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("ContractTurnInBtn"));
		m_ContractRefreshBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("ContractRefreshBtn"));
		m_ContractInfoText = TextWidget.Cast(layoutRoot.FindAnyWidget("ContractInfoText"));

		m_PanelStorage = layoutRoot.FindAnyWidget("PanelStorage");
		m_StorageHeaderText = TextWidget.Cast(layoutRoot.FindAnyWidget("StorageHeaderText"));
		m_HdrStorageItem = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrStorageItem"));
		m_HdrStorageQty = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrStorageQty"));
		m_HdrStorageWho = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrStorageWho"));
		m_HdrStorageDate = TextWidget.Cast(layoutRoot.FindAnyWidget("HdrStorageDate"));
		m_StorageList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("StorageList"));
		m_StorageTilesRoot = layoutRoot.FindAnyWidget("StorageTilesRoot");
		m_StorageInfoText = TextWidget.Cast(layoutRoot.FindAnyWidget("StorageInfoText"));
		m_DepositToStorageBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("DepositToStorageBtn"));
		m_TakeFromStorageBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("TakeFromStorageBtn"));
		m_StorageHintText = TextWidget.Cast(layoutRoot.FindAnyWidget("StorageHintText"));
		m_StoragePreview = ItemPreviewWidget.Cast(layoutRoot.FindAnyWidget("StoragePreview"));
		m_StorageSearchEdit = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("StorageSearchEdit"));
		m_StorageSearchHint = TextWidget.Cast(layoutRoot.FindAnyWidget("StorageSearchHint"));

		m_PanelDescEdit = layoutRoot.FindAnyWidget("PanelDescEdit");
		m_DescTitleText = TextWidget.Cast(layoutRoot.FindAnyWidget("DescTitleText"));
		m_DescEdit = MultilineEditBoxWidget.Cast(layoutRoot.FindAnyWidget("DescEdit"));
		m_DescViewText = TextWidget.Cast(layoutRoot.FindAnyWidget("DescViewText"));
		m_SaveDescBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("SaveDescBtn"));
		m_CloseDescBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("CloseDescBtn"));
		m_DescHintText = TextWidget.Cast(layoutRoot.FindAnyWidget("DescHintText"));

		m_PanelPerms = layoutRoot.FindAnyWidget("PanelPerms");
		m_PermsHeaderText = TextWidget.Cast(layoutRoot.FindAnyWidget("PermsHeaderText"));
		m_PermsHdrAction = TextWidget.Cast(layoutRoot.FindAnyWidget("PermsHdrAction"));
		m_PermsHdrRank = TextWidget.Cast(layoutRoot.FindAnyWidget("PermsHdrRank"));
		m_PermsList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("PermsList"));
		m_PermRankUpBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("PermRankUpBtn"));
		m_PermRankDownBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("PermRankDownBtn"));
		m_PermsCloseBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("PermsCloseBtn"));
		m_PermsHintText = TextWidget.Cast(layoutRoot.FindAnyWidget("PermsHintText"));
		m_WebhookStatusText = TextWidget.Cast(layoutRoot.FindAnyWidget("WebhookStatusText"));
		m_WebhookEditBg = layoutRoot.FindAnyWidget("WebhookEditBg");
		m_WebhookEdit = EditBoxWidget.Cast(layoutRoot.FindAnyWidget("WebhookEdit"));
		m_WebhookSaveBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("WebhookSaveBtn"));
		m_WebhookClearBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("WebhookClearBtn"));
		m_WebhookTestBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("WebhookTestBtn"));
		m_WebhookAuditBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("WebhookAuditBtn"));

		m_PanelClanInfo = layoutRoot.FindAnyWidget("PanelClanInfo");
		m_InfoTitleText = TextWidget.Cast(layoutRoot.FindAnyWidget("InfoTitleText"));
		m_InfoDescLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("InfoDescLabel"));
		m_InfoDescText = TextWidget.Cast(layoutRoot.FindAnyWidget("InfoDescText"));
		m_InfoMembersLabel = TextWidget.Cast(layoutRoot.FindAnyWidget("InfoMembersLabel"));
		m_InfoHdrName = TextWidget.Cast(layoutRoot.FindAnyWidget("InfoHdrName"));
		m_InfoHdrRank = TextWidget.Cast(layoutRoot.FindAnyWidget("InfoHdrRank"));
		m_InfoHdrTitle = TextWidget.Cast(layoutRoot.FindAnyWidget("InfoHdrTitle"));
		m_InfoHdrStatus = TextWidget.Cast(layoutRoot.FindAnyWidget("InfoHdrStatus"));
		m_InfoMembersList = TextListboxWidget.Cast(layoutRoot.FindAnyWidget("InfoMembersList"));
		m_ApplyBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("ApplyBtn"));
		m_AdminEnterClanBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("AdminEnterClanBtn"));
		m_CloseInfoBtn = ButtonWidget.Cast(layoutRoot.FindAnyWidget("CloseInfoBtn"));

		m_AllButtons.Clear();
		CollectButtons(layoutRoot);

		layoutRoot.Update();

		AlignColumnHeaders();

		SetLabels();
		ApplyRedesignAssets();

		ApplyButtonStyles();

		return layoutRoot;
	}

	protected void CollectButtons(Widget root)
	{
		if (!root)
			return;

		Widget child = root.GetChildren();
		while (child)
		{
			ButtonWidget button = ButtonWidget.Cast(child);
			if (button)
				m_AllButtons.Insert(button);

			CollectButtons(child);
			child = child.GetSibling();
		}
	}

	protected void ApplyButtonStyles()
	{
		foreach (ButtonWidget button : m_AllButtons)
			SetButtonNormal(button);
	}

	protected void SetButtonText(ButtonWidget button, string text)
	{
		if (!button)
			return;
		button.SetText(SM_PartyLoc.Text(text));
		button.SetTextColor(ARGB(255, 255, 255, 242));
	}

	protected void ShowBtnDressing(string btnName, bool show)
	{
		Widget bg = layoutRoot.FindAnyWidget(btnName + "Bg");
		Widget ac = layoutRoot.FindAnyWidget(btnName + "Accent");
		if (bg) bg.Show(show);
		if (ac) ac.Show(show);
	}

	protected void ShowEditUnderline(string editName, bool show)
	{
		Widget ul = layoutRoot.FindAnyWidget(editName + "Underline");
		if (ul) ul.Show(show);
	}

	protected void SetBtnDressingState(string btnName, bool enabled)
	{
		Widget bg = layoutRoot.FindAnyWidget(btnName + "Bg");
		Widget ac = layoutRoot.FindAnyWidget(btnName + "Accent");
		ButtonWidget btn = ButtonWidget.Cast(layoutRoot.FindAnyWidget(btnName));

		if (enabled)
		{
			if (bg)
				bg.SetColor(ARGB(230, 50, 43, 35));
			if (ac)
				ac.SetColor(ARGB(230, 184, 115, 51));
			if (btn)
				btn.SetTextColor(ARGB(255, 255, 255, 242));
		}
		else
		{
			if (bg)
				bg.SetColor(ARGB(150, 41, 35, 29));
			if (ac)
				ac.SetColor(ARGB(120, 158, 145, 126));
			if (btn)
				btn.SetTextColor(ARGB(180, 171, 157, 137));
		}
	}

	protected void LoadMenuImage(ImageWidget image, string path, float alpha = 1.0)
	{
		if (!image)
			return;

		image.LoadImageFile(0, path);
		image.SetImage(0);
		image.SetAlpha(alpha);
	}

	protected void SetActionText(ButtonWidget button, TextWidget label, string text)
	{
		if (button)
			button.SetText(SM_PartyLoc.Text(""));
		if (label)
			label.SetText(SM_PartyLoc.Text(text));
	}

	protected void SetTabText(int tabIndex, ButtonWidget button, string text)
	{
		if (button)
			button.SetText(SM_PartyLoc.Text(""));

		if (tabIndex >= 0 && tabIndex < m_TabLabels.Count() && m_TabLabels[tabIndex])
		{
			m_TabLabels[tabIndex].SetText(SM_PartyLoc.Text(text));
		}
		else if (button)
		{
			button.SetText(SM_PartyLoc.Text(text));
		}
	}

	protected void LoadTabIcon(int tabIndex, string path)
	{
		if (tabIndex < 0 || tabIndex >= m_TabIcons.Count())
			return;

		LoadMenuImage(m_TabIcons[tabIndex], path, 0.82);
	}

	protected void UpdateTabVisuals()
	{
		for (int i = 0; i < m_TabLabels.Count(); i++)
		{
			int labelColor = ARGB(190, 176, 162, 141);
			int iconColor = ARGB(175, 207, 173, 105);
			if (i == m_CurrentTab)
			{
				labelColor = ARGB(255, 250, 240, 228);
				iconColor = ARGB(255, 249, 200, 94);
			}

			if (m_TabLabels[i])
				m_TabLabels[i].SetColor(labelColor);
			if (i < m_TabIcons.Count() && m_TabIcons[i])
				m_TabIcons[i].SetColor(iconColor);
		}

		for (int p = 0; p < m_TabPills.Count(); p++)
		{
			if (m_TabPills[p])
				m_TabPills[p].Show(p == m_CurrentTab);
		}
	}

	protected void ApplyRedesignAssets()
	{
		LoadTabIcon(0, "SM_PartyMod\\GUI\\icons\\nav_my_clan.paa");
		LoadTabIcon(1, "SM_PartyMod\\GUI\\icons\\nav_clans.paa");
		LoadTabIcon(2, "SM_PartyMod\\GUI\\icons\\nav_invites.paa");
		LoadTabIcon(3, "SM_PartyMod\\GUI\\icons\\nav_tops.paa");
		LoadTabIcon(4, "SM_PartyMod\\GUI\\icons\\nav_treasury.paa");
		LoadTabIcon(5, "SM_PartyMod\\GUI\\icons\\nav_log.paa");
		LoadTabIcon(7, "SM_PartyMod\\GUI\\icons\\nav_market.paa");
		LoadTabIcon(8, "SM_PartyMod\\GUI\\icons\\nav_server.paa");
		LoadTabIcon(9, "SM_PartyMod\\GUI\\icons\\nav_storage.paa");
		LoadTabIcon(10, "SM_PartyMod\\GUI\\icons\\nav_auction.paa");
		LoadTabIcon(11, "SM_PartyMod\\GUI\\icons\\nav_achievements.paa");
		LoadTabIcon(12, "SM_PartyMod\\GUI\\icons\\nav_settings.paa");
		LoadTabIcon(13, "SM_PartyMod\\GUI\\icons\\nav_tops.paa");
		LoadTabIcon(14, "SM_PartyMod\\GUI\\icons\\nav_auction.paa");
		LoadTabIcon(15, "SM_PartyMod\\GUI\\icons\\nav_log.paa");

		LoadMenuImage(m_MenuBgImage, "SM_PartyMod\\GUI\\textures\\window_bg.paa", 1.0);
		LoadMenuImage(m_OnlineIcon, "SM_PartyMod\\GUI\\icons\\nav_clans.paa", 0.95);

		LoadMenuImage(m_MembersWatermarkLogo, "SM_PartyMod\\GUI\\logo\\logo.paa", 0.07);
		LoadMenuImage(m_MembersNoiseImage, "SM_PartyMod\\GUI\\textures\\panel_noise.paa", 0.16);
		LoadMenuImage(m_ActionsNoiseImage, "SM_PartyMod\\GUI\\textures\\panel_noise.paa", 0.14);

		LoadMenuImage(m_InviteIcon, "SM_PartyMod\\GUI\\icons\\invite.paa", 0.95);
		LoadMenuImage(m_KickIcon, "SM_PartyMod\\GUI\\icons\\kick.paa", 0.9);
		LoadMenuImage(m_PromoteIcon, "SM_PartyMod\\GUI\\icons\\promote.paa", 0.9);
		LoadMenuImage(m_DemoteIcon, "SM_PartyMod\\GUI\\icons\\demote.paa", 0.9);
		LoadMenuImage(m_LeaveIcon, "SM_PartyMod\\GUI\\icons\\leave.paa", 0.9);
		LoadMenuImage(m_DisbandIcon, "SM_PartyMod\\GUI\\icons\\disband.paa", 0.9);
		LoadMenuImage(m_DescIcon, "SM_PartyMod\\GUI\\icons\\description.paa", 0.9);
		LoadMenuImage(m_PermsIcon, "SM_PartyMod\\GUI\\icons\\permissions.paa", 0.9);
		LoadMenuImage(m_AdminLeaveClanIcon, "SM_PartyMod\\GUI\\icons\\admin_leave.paa", 0.9);

		for (int c = 0; c < m_ColorSwatches.Count(); c++)
		{
			ImageWidget marker = ImageWidget.Cast(m_ColorSwatches[c]);
			if (marker)
			{
				LoadMenuImage(marker, "SM_PartyMod\\GUI\\textures\\color_marker.paa", 1.0);
				marker.SetColor(SM_ClanColors.GetColor(c));
			}

			ImageWidget selected = ImageWidget.Cast(m_ColorSelected[c]);
			if (selected)
			{
				LoadMenuImage(selected, "SM_PartyMod\\GUI\\textures\\color_selected.paa", 0.0);
				selected.Show(false);
			}
		}
	}

	protected void PositionHeaders(string listName, array<string> names, array<float> startFractions)
	{
		float scrollbar = 22;
		float inset = 3;

		Widget list = layoutRoot.FindAnyWidget(listName);
		if (!list)
			return;

		float lx, ly, lw, lh;
		list.GetPos(lx, ly);
		list.GetSize(lw, lh);
		if (lw < 50)
			return;
		float effective = lw - scrollbar;
		if (effective < 1)
			effective = lw;

		for (int i = 0; i < names.Count() && i < startFractions.Count(); i++)
		{
			Widget h = layoutRoot.FindAnyWidget(names[i]);
			if (!h)
				continue;
			float hx, hy;
			h.GetPos(hx, hy);
			h.SetPos(lx + startFractions[i] * effective + inset, hy);
		}
	}

	protected void PositionColumnSeparators(string listName, array<string> names, array<float> startFractions)
	{
		float scrollbar = 22;

		Widget list = layoutRoot.FindAnyWidget(listName);
		if (!list)
			return;

		float lx, ly, lw, lh;
		list.GetPos(lx, ly);
		list.GetSize(lw, lh);
		if (lw < 50)
			return;

		float effective = lw - scrollbar;
		if (effective < 1)
			effective = lw;

		for (int i = 0; i < names.Count() && i < startFractions.Count(); i++)
		{
			Widget sep = layoutRoot.FindAnyWidget(names[i]);
			if (!sep)
				continue;

			float sx, sy;
			sep.GetPos(sx, sy);
			sep.SetPos(lx + startFractions[i] * effective, sy);
		}
	}

	protected void AlignColumnHeaders()
	{
		array<string> memH = {"HdrName", "HdrRank", "HdrMemberTitle", "HdrStatus", "HdrHP"};
		array<float>  memF = {0.0, 0.32, 0.49, 0.71, 0.88};
		PositionHeaders("MembersList", memH, memF);
		array<string> memS = {"MembersColSep1", "MembersColSep2", "MembersColSep3", "MembersColSep4"};
		array<float>  memSF = {0.32, 0.49, 0.71, 0.88};
		PositionColumnSeparators("MembersList", memS, memSF);

		array<string> clanH = {"HdrClanName", "HdrClanTag", "HdrClanCount", "HdrClanLeader", "HdrClanLeaderTitle"};
		array<float>  clanF = {0.0, 0.34, 0.46, 0.58, 0.78};
		PositionHeaders("ClansList", clanH, clanF);
		array<string> clanS = {"ClansColSep1", "ClansColSep2", "ClansColSep3", "ClansColSep4"};
		array<float>  clanSF = {0.34, 0.46, 0.58, 0.78};
		PositionColumnSeparators("ClansList", clanS, clanSF);

		array<string> invH = {"HdrInvClan", "HdrInvFrom"};
		array<float>  invF = {0.0, 0.52};
		PositionHeaders("InvitesList", invH, invF);
		array<string> invS = {"InvitesColSep1"};
		array<float>  invSF = {0.52};
		PositionColumnSeparators("InvitesList", invS, invSF);

		array<string> topH = {"TopsHdrPlace", "TopsHdrClan", "TopsHdrValue"};
		array<float>  topF = {0.0, 0.06, 0.56};
		PositionHeaders("TopsList", topH, topF);

		array<string> infH = {"InfoHdrName", "InfoHdrRank", "InfoHdrTitle", "InfoHdrStatus"};
		array<float>  infF = {0.0, 0.34, 0.52, 0.76};
		PositionHeaders("InfoMembersList", infH, infF);

		array<string> mrkH = {"HdrMarketItem", "HdrMarketQty", "HdrMarketPrice", "HdrMarketClan"};
		array<float>  mrkF = {0.02, 0.54, 0.70, 0.87};
		PositionHeaders("MarketList", mrkH, mrkF);
		array<string> mrkS = {"MarketColSep1", "MarketColSep2", "MarketColSep3"};
		array<float>  mrkSF = {0.53, 0.69, 0.86};
		PositionColumnSeparators("MarketList", mrkS, mrkSF);

		array<string> srvH = {"HdrServerMarketItem", "HdrServerMarketQty", "HdrServerMarketPrice", "HdrServerMarketLimit"};
		array<float>  srvF = {0.02, 0.54, 0.70, 0.87};
		PositionHeaders("ServerMarketList", srvH, srvF);
		array<string> srvS = {"ServerMarketColSep1", "ServerMarketColSep2", "ServerMarketColSep3"};
		array<float>  srvSF = {0.53, 0.69, 0.86};
		PositionColumnSeparators("ServerMarketList", srvS, srvSF);

		array<string> aucH = {"HdrAuctionItem", "HdrAuctionStart", "HdrAuctionBid", "HdrAuctionTime", "HdrAuctionSeller"};
		array<float>  aucF = {0.02, 0.54, 0.70, 0.87, 0.96};
		PositionHeaders("AuctionList", aucH, aucF);
		array<string> aucS = {"AuctionColSep1", "AuctionColSep2", "AuctionColSep3", "AuctionColSep4"};
		array<float>  aucSF = {0.53, 0.69, 0.86, 0.96};
		PositionColumnSeparators("AuctionList", aucS, aucSF);

		array<string> achH = {"HdrAchName", "HdrAchProgress", "HdrAchReward"};
		array<float>  achF = {0.0, 0.52, 0.72};
		PositionHeaders("AchievementsList", achH, achF);
		array<string> achS = {"AchievementsColSep1", "AchievementsColSep2"};
		array<float>  achSF = {0.52, 0.72};
		PositionColumnSeparators("AchievementsList", achS, achSF);

		array<string> titleH = {"HdrPlayerTitlePlace", "HdrPlayerTitleName", "HdrPlayerTitleTitle", "HdrPlayerTitleXP", "HdrPlayerTitleClan"};
		array<float>  titleF = {0.0, 0.08, 0.42, 0.68, 0.82};
		PositionHeaders("PlayerTitlesList", titleH, titleF);
		array<string> titleS = {"PlayerTitlesColSep1", "PlayerTitlesColSep2", "PlayerTitlesColSep3", "PlayerTitlesColSep4"};
		array<float>  titleSF = {0.08, 0.42, 0.68, 0.82};
		PositionColumnSeparators("PlayerTitlesList", titleS, titleSF);

		array<string> stoH = {"HdrStorageItem", "HdrStorageQty", "HdrStorageWho", "HdrStorageDate"};
		array<float>  stoF = {0.02, 0.54, 0.70, 0.87};
		PositionHeaders("StorageList", stoH, stoF);

		array<string> stoS = {"StorageColSep1", "StorageColSep2", "StorageColSep3"};
		array<float>  stoSF = {0.53, 0.69, 0.86};
		PositionColumnSeparators("StorageList", stoS, stoSF);

		array<string> adminChatH = {"AdminChatHdrTime", "AdminChatHdrChannel", "AdminChatHdrPlayer", "AdminChatHdrMessage"};
		array<float>  adminChatF = {0.0, 0.12, 0.24, 0.46};
		PositionHeaders("AdminChatList", adminChatH, adminChatF);
		array<string> adminChatS = {"AdminChatColSep1", "AdminChatColSep2", "AdminChatColSep3"};
		array<float>  adminChatSF = {0.12, 0.24, 0.46};
		PositionColumnSeparators("AdminChatList", adminChatS, adminChatSF);

		array<string> onlineH = {"OnlineHdrName", "OnlineHdrClan", "OnlineHdrTitle"};
		array<float>  onlineF = {0.0, 0.44, 0.70};
		PositionHeaders("OnlineList", onlineH, onlineF);
		array<string> onlineS = {"OnlineColSep1", "OnlineColSep2"};
		array<float>  onlineSF = {0.44, 0.70};
		PositionColumnSeparators("OnlineList", onlineS, onlineSF);

		array<string> permH = {"PermsHdrAction", "PermsHdrRank"};
		array<float>  permF = {0.0, 0.70};
		PositionHeaders("PermsList", permH, permF);
		array<string> permS = {"PermsColSep"};
		array<float>  permSF = {0.70};
		PositionColumnSeparators("PermsList", permS, permSF);
	}

	protected bool IsMenuButton(Widget w)
	{
		foreach (ButtonWidget button : m_AllButtons)
		{
			if (w == button)
				return true;
		}
		return false;
	}

	protected bool IsColorButton(Widget w)
	{
		foreach (ButtonWidget cb : m_ColorBtns)
		{
			if (w == cb)
				return true;
		}
		return false;
	}

	protected void SetButtonNormal(Widget w)
	{
		if (!w)
			return;

		if (IsColorButton(w))
		{
			w.SetColor(ARGB(0, 0, 0, 0));
			return;
		}

		if (w == m_OnlineBtn)
		{
			w.SetColor(ARGB(0, 0, 0, 0));
			return;
		}

		w.SetColor(ARGB(90, 50, 42, 35));
		ButtonWidget button = ButtonWidget.Cast(w);
		if (button)
			button.SetTextColor(ARGB(255, 255, 255, 242));
	}

	protected ButtonWidget GetActiveTabButton()
	{
		switch (m_CurrentTab)
		{
			case 0: return m_TabMyClanBtn;
			case 1: return m_TabClansBtn;
			case 2: return m_TabInvitesBtn;
			case 3: return m_TabTopsBtn;
			case 4: return m_TabTreasuryBtn;
			case 5: return m_TabLogBtn;
			case 6: return m_TabMapBtn;
			case 7: return m_TabMarketBtn;
			case 8: return m_TabServerMarketBtn;
			case 9: return m_TabStorageBtn;
			case 10: return m_TabAuctionBtn;
			case 11: return m_TabAchievementsBtn;
			case 12: return m_TabSettingsBtn;
			case 13: return m_TabTitlesBtn;
			case 14: return m_TabContractsBtn;
			case 15: return m_TabAdminChatBtn;
		}
		return null;
	}

	protected void SetButtonHighlight(Widget w)
	{
		if (!w)
			return;

		if (IsColorButton(w))
		{
			w.SetColor(ARGB(0, 0, 0, 0));
			return;
		}

		if (w == m_OnlineBtn)
		{
			w.SetColor(ARGB(55, 50, 42, 35));
			return;
		}

		if (w == GetActiveTabButton())
		{
			SetButtonNormal(w);
			return;
		}

		w.SetColor(ARGB(240, 50, 42, 35));
		ButtonWidget button = ButtonWidget.Cast(w);
		if (button)
			button.SetTextColor(ARGB(255, 255, 255, 242));
	}

	protected bool IsSettingToggleWidget(Widget w)
	{
		if (!w)
			return false;

		for (int i = 0; i < m_SettingToggles.Count(); i++)
		{
			if (w == m_SettingToggles[i])
				return true;
		}

		return false;
	}

	protected void ClearSettingToggleFrame(Widget w)
	{
		if (!w)
			return;

		w.SetColor(ARGB(0, 0, 0, 0));
		ButtonWidget button = ButtonWidget.Cast(w);
		if (button)
			button.SetTextColor(ARGB(0, 0, 0, 0));
		SetFocus(NULL);
	}

	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (IsSettingToggleWidget(w))
		{
			ClearSettingToggleFrame(w);
			return true;
		}

		if (w == m_CreateClanBtn)
		{
			SetCreateButtonHover();
			return true;
		}

		if (IsMenuButton(w))
		{
			SetButtonHighlight(w);
			return true;
		}
		return super.OnMouseEnter(w, x, y);
	}

	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (IsSettingToggleWidget(w))
		{
			ClearSettingToggleFrame(w);
			return true;
		}

		if (w == m_CreateClanBtn)
		{
			UpdateCreateCounters();
			return true;
		}

		if (IsMenuButton(w))
		{
			SetButtonNormal(w);
			return true;
		}
		return super.OnMouseLeave(w, enterW, x, y);
	}

	override bool OnFocus(Widget w, int x, int y)
	{
		if (IsSettingToggleWidget(w))
		{
			ClearSettingToggleFrame(w);
			return true;
		}

		if (w == m_CreateClanBtn)
		{
			SetCreateButtonHover();
			return true;
		}

		if (IsMenuButton(w))
		{
			SetButtonHighlight(w);
			return true;
		}

		if (w)
		{
			Widget ul = layoutRoot.FindAnyWidget(w.GetName() + "Underline");
			if (ul)
				ul.SetColor(ARGB(255, 184, 115, 51));
		}
		return super.OnFocus(w, x, y);
	}

	override bool OnFocusLost(Widget w, int x, int y)
	{
		if (IsSettingToggleWidget(w))
		{
			ClearSettingToggleFrame(w);
			return true;
		}

		if (w == m_CreateClanBtn)
		{
			UpdateCreateCounters();
			return true;
		}

		if (IsMenuButton(w))
		{
			SetButtonNormal(w);
			return true;
		}

		if (w)
		{
			Widget ul = layoutRoot.FindAnyWidget(w.GetName() + "Underline");
			if (ul)
				ul.SetColor(ARGB(115, 184, 115, 51));
		}
		return super.OnFocusLost(w, x, y);
	}

	protected void SetLabels()
	{
		m_TitleText.SetText(SM_PartyLoc.Text("#STR_SMP_00565"));
		if (m_TitleLogo)
		{
			m_TitleLogo.LoadImageFile(0, "SM_PartyMod\\GUI\\logo\\logo.paa");
			m_TitleLogo.SetImage(0);
		}
		m_CloseBtn.SetText(SM_PartyLoc.Text("X"));
		if (m_OnlineText)
			m_OnlineText.SetText(SM_PartyLoc.Text("#STR_SMP_00727"));
		if (m_OnlineValueText)
			m_OnlineValueText.SetText(SM_PartyLoc.Text(SM_ClanClientData.OnlinePlayers.Count().ToString()));
		if (m_OnlineHeaderText)
			m_OnlineHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_00471"));
		if (m_OnlineCloseBtn)
			m_OnlineCloseBtn.SetText(SM_PartyLoc.Text("X"));
		SetTabText(0, m_TabMyClanBtn, "#STR_SMP_00649");
		SetTabText(1, m_TabClansBtn, "#STR_SMP_00341");
		SetTabText(2, m_TabInvitesBtn, "#STR_SMP_00842");
		SetTabText(3, m_TabTopsBtn, "#STR_SMP_01003");
		SetTabText(4, m_TabTreasuryBtn, "#STR_SMP_00482");
		SetTabText(5, m_TabLogBtn, "#STR_SMP_00434");
		if (m_TabSettingsBtn)
			SetTabText(12, m_TabSettingsBtn, "#STR_SMP_00674");
		TextWidget settingsHdr = TextWidget.Cast(layoutRoot.FindAnyWidget("SettingsHeaderText"));
		if (settingsHdr)
			settingsHdr.SetText(SM_PartyLoc.Text("#STR_SMP_00674"));
		TextWidget settingsHint = TextWidget.Cast(layoutRoot.FindAnyWidget("SettingsHintText"));
		if (settingsHint)
			settingsHint.SetText(SM_PartyLoc.Text("#STR_SMP_00219"));
		TextWidget membersHdr = TextWidget.Cast(layoutRoot.FindAnyWidget("MembersHeaderText"));
		if (membersHdr)
			membersHdr.SetText(SM_PartyLoc.Text("#STR_SMP_00800"));
		if (m_MembersCloseBtn)
			m_MembersCloseBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00441"));
		if (m_TabMapBtn)
			SetTabText(6, m_TabMapBtn, "#STR_SMP_00498");
		if (m_TabAuctionBtn)
			SetTabText(10, m_TabAuctionBtn, "#STR_SMP_00258");
		if (m_TabAchievementsBtn)
			SetTabText(11, m_TabAchievementsBtn, "#STR_SMP_00426");
		if (m_TabTitlesBtn)
			SetTabText(13, m_TabTitlesBtn, "#STR_SMP_00458");
		if (m_TabContractsBtn)
			SetTabText(14, m_TabContractsBtn, "#STR_SMP_00583");
		if (m_TabAdminChatBtn)
			SetTabText(15, m_TabAdminChatBtn, "#STR_SMP_00246");

		m_NoClanInfoText.SetText(SM_PartyLoc.Text("#STR_SMP_00349"));
		m_CreateNameLabel.SetText(SM_PartyLoc.Text("#STR_SMP_00665"));
		m_CreateTagLabel.SetText(SM_PartyLoc.Text("#STR_SMP_00985"));
		m_CreateClanBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00963"));

		m_HdrName.SetText(SM_PartyLoc.Text("#STR_SMP_00477"));
		m_HdrRank.SetText(SM_PartyLoc.Text("#STR_SMP_00882"));
		if (m_HdrMemberTitle)
			m_HdrMemberTitle.SetText(SM_PartyLoc.Text("#STR_SMP_00457"));
		m_HdrStatus.SetText(SM_PartyLoc.Text("#STR_SMP_00343"));
		m_HdrHP.SetText(SM_PartyLoc.Text("#STR_SMP_00977"));
		SetActionText(m_InviteBtn, m_InviteActionText, "#STR_SMP_00836");
		SetActionText(m_KickBtn, m_KickActionText, "#STR_SMP_00502");
		SetActionText(m_PromoteBtn, m_PromoteActionText, "#STR_SMP_00789");
		SetActionText(m_DemoteBtn, m_DemoteActionText, "#STR_SMP_00810");
		SetActionText(m_LeaveBtn, m_LeaveActionText, "#STR_SMP_00801");
		SetActionText(m_DisbandBtn, m_DisbandActionText, "#STR_SMP_00886");
		if (m_DisbandConfirmTitle)
			m_DisbandConfirmTitle.SetText(SM_PartyLoc.Text("#STR_SMP_00887"));
		if (m_DisbandConfirmYesBtn)
			SetButtonText(m_DisbandConfirmYesBtn, "#STR_SMP_00885");
		if (m_DisbandConfirmNoBtn)
			SetButtonText(m_DisbandConfirmNoBtn, "#STR_SMP_00757");
		SetActionText(m_DescBtn, m_DescActionText, "#STR_SMP_00731");
		SetActionText(m_PermsBtn, m_PermsActionText, "#STR_SMP_00822");
		SetActionText(m_AdminLeaveClanBtn, m_AdminLeaveClanActionText, "#STR_SMP_00375");
		m_ColorLabel.SetText(SM_PartyLoc.Text("#STR_SMP_01049"));

		m_InviteTitleText.SetText(SM_PartyLoc.Text("#STR_SMP_00355"));
		m_ConfirmInviteBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00835"));
		m_RefreshPlayersBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00716"));
		m_CancelInviteBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00662"));

		m_ClansHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_00566"));
		m_HdrClanName.SetText(SM_PartyLoc.Text("#STR_SMP_00663"));
		m_HdrClanTag.SetText(SM_PartyLoc.Text("#STR_SMP_00984"));
		m_HdrClanCount.SetText(SM_PartyLoc.Text("#STR_SMP_00965"));
		m_HdrClanLeader.SetText(SM_PartyLoc.Text("#STR_SMP_00599"));
		if (m_HdrClanLeaderTitle)
			m_HdrClanLeaderTitle.SetText(SM_PartyLoc.Text("#STR_SMP_00457"));
		m_RefreshClansBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00715"));

		m_InvitesHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_00301"));
		m_HdrInvClan.SetText(SM_PartyLoc.Text("#STR_SMP_00505"));
		m_HdrInvFrom.SetText(SM_PartyLoc.Text("#STR_SMP_00591"));
		m_AcceptInviteBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00844"));
		m_DeclineInviteBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00752"));

		for (int t = 0; t < SM_TopCategory.COUNT; t++)
		{
			if (m_TopCatBtns[t])
				m_TopCatBtns[t].SetText(SM_PartyLoc.Text(SM_TopCategory.GetShortName(t)));
		}
		m_TopsHdrPlace.SetText(SM_PartyLoc.Text("#"));
		m_TopsHdrClan.SetText(SM_PartyLoc.Text("#STR_SMP_00505"));
		m_TopsHdrValue.SetText(SM_PartyLoc.Text("#STR_SMP_00461"));

		m_TreasuryBalanceText.SetText(SM_PartyLoc.Text("#STR_SMP_00484"));
		m_DepositBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00329"));
		m_DepositAmountBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00330"));
		m_WithdrawLabel.SetText(SM_PartyLoc.Text("#STR_SMP_00954"));
		m_WithdrawBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00953"));
		m_UpgradeClanBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00874"));
		m_TreasuryRatesHeader.SetText(SM_PartyLoc.Text("#STR_SMP_00596"));
		m_DepositHeader.SetText(SM_PartyLoc.Text("#STR_SMP_00816"));
		m_WithdrawHeader.SetText(SM_PartyLoc.Text("#STR_SMP_00952"));
		m_LevelHeader.SetText(SM_PartyLoc.Text("#STR_SMP_01041"));

		m_LogHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_00435"));
		if (m_AdminChatHeaderText)
			m_AdminChatHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_00242"));
		if (m_AdminChatHintText)
			m_AdminChatHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00400"));
		if (m_AdminChatRefreshBtn)
			m_AdminChatRefreshBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00715"));
		if (m_AdminChatMuteTitleText)
			m_AdminChatMuteTitleText.SetText(SM_PartyLoc.Text("#STR_SMP_00652"));
		if (m_AdminChatMuteMinutesLabel)
			m_AdminChatMuteMinutesLabel.SetText(SM_PartyLoc.Text("#STR_SMP_00648"));
		if (m_AdminChatMuteApplyBtn)
			m_AdminChatMuteApplyBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00371"));
		if (m_AdminChatUnmuteBtn)
			m_AdminChatUnmuteBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00880"));
		if (m_AdminChatMuteCloseBtn)
			m_AdminChatMuteCloseBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00441"));

		m_MapHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_00500"));
		m_MapHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00224"));
		m_MapCloseBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00442"));
		if (m_MapModePrevBtn)
			m_MapModePrevBtn.SetText(SM_PartyLoc.Text("<"));
		if (m_MapModeNextBtn)
			m_MapModeNextBtn.SetText(SM_PartyLoc.Text(">"));
		if (m_MapModeLabel)
			m_MapModeLabel.SetText(SM_PartyLoc.Text("#STR_SMP_00611"));
		m_MapMarkerNameLabel.SetText(SM_PartyLoc.Text("#STR_SMP_00667"));
		m_MapAddMarkerBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00417"));
		m_MapRemoveMarkerBtn.SetText(SM_PartyLoc.Text("#STR_SMP_01037"));
		m_MapSelectedText.SetText(SM_PartyLoc.Text("#STR_SMP_01010"));
		m_MapColorLabel.SetText(SM_PartyLoc.Text("#STR_SMP_01050"));
		m_MapColorBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00938"));
		if (m_MapShowMeBtn)
			m_MapShowMeBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00797"));
		if (m_MapAdminBasesBtn)
			m_MapAdminBasesBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00271"));
		SetTabText(7, m_TabMarketBtn, "#STR_SMP_00891");
		m_HdrMarketItem.SetText(SM_PartyLoc.Text("#STR_SMP_00827"));
		m_HdrMarketQty.SetText(SM_PartyLoc.Text("#STR_SMP_00569"));
		m_HdrMarketPrice.SetText(SM_PartyLoc.Text("#STR_SMP_01058"));
		m_HdrMarketClan.SetText(SM_PartyLoc.Text("#STR_SMP_00505"));
		m_BuyLotBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00594"));
		m_CancelLotBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00955"));
		m_SellLabel.SetText(SM_PartyLoc.Text("#STR_SMP_00855"));
		m_SellLotBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00379"));
		if (m_MarketSearchHint)
			m_MarketSearchHint.SetText(SM_PartyLoc.Text("#STR_SMP_00793"));

		SetTabText(8, m_TabServerMarketBtn, "#STR_SMP_00909");
		m_HdrServerMarketItem.SetText(SM_PartyLoc.Text("#STR_SMP_00827"));
		m_HdrServerMarketQty.SetText(SM_PartyLoc.Text("#STR_SMP_00743"));
		m_HdrServerMarketPrice.SetText(SM_PartyLoc.Text("#STR_SMP_01058"));
		m_HdrServerMarketLimit.SetText(SM_PartyLoc.Text("#STR_SMP_00601"));
		m_BuyServerMarketBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00594"));
		if (m_ServerMarketSearchHint)
			m_ServerMarketSearchHint.SetText(SM_PartyLoc.Text("#STR_SMP_00793"));

		if (m_TabAuctionBtn)
			SetTabText(10, m_TabAuctionBtn, "#STR_SMP_00258");
		if (m_HdrAuctionItem)
			m_HdrAuctionItem.SetText(SM_PartyLoc.Text("#STR_SMP_00827"));
		if (m_HdrAuctionStart)
			m_HdrAuctionStart.SetText(SM_PartyLoc.Text("#STR_SMP_00972"));
		if (m_HdrAuctionBid)
			m_HdrAuctionBid.SetText(SM_PartyLoc.Text("#STR_SMP_00969"));
		if (m_HdrAuctionTime)
			m_HdrAuctionTime.SetText(SM_PartyLoc.Text("#STR_SMP_00741"));
		if (m_HdrAuctionSeller)
			m_HdrAuctionSeller.SetText(SM_PartyLoc.Text("#STR_SMP_00854"));
		if (m_AuctionBidBtn)
			m_AuctionBidBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00907"));
		if (m_AuctionCancelBtn)
			m_AuctionCancelBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00955"));
		if (m_AuctionStartLabel)
			m_AuctionStartLabel.SetText(SM_PartyLoc.Text("#STR_SMP_00973"));
		if (m_AuctionStepLabel)
			m_AuctionStepLabel.SetText(SM_PartyLoc.Text("#STR_SMP_01071"));
		if (m_AuctionDurationLabel)
			m_AuctionDurationLabel.SetText(SM_PartyLoc.Text("#STR_SMP_00646"));
		if (m_AuctionSellBtn)
			m_AuctionSellBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00379"));
		if (m_AuctionSearchHint)
			m_AuctionSearchHint.SetText(SM_PartyLoc.Text("#STR_SMP_00793"));

		if (m_TabAchievementsBtn)
			SetTabText(11, m_TabAchievementsBtn, "#STR_SMP_00426");
		if (m_AchPersonalBtn)
			m_AchPersonalBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00611"));
		if (m_AchClanBtn)
			m_AchClanBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00563"));
		if (m_HdrAchName)
			m_HdrAchName.SetText(SM_PartyLoc.Text("#STR_SMP_00421"));
		if (m_HdrAchProgress)
			m_HdrAchProgress.SetText(SM_PartyLoc.Text("#STR_SMP_00851"));
		if (m_HdrAchReward)
			m_HdrAchReward.SetText(SM_PartyLoc.Text("#STR_SMP_00656"));
		if (m_ClaimAchievementBtn)
			m_ClaimAchievementBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00438"));

		if (m_PlayerTitlesHeaderText)
			m_PlayerTitlesHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_00460"));
		if (m_HdrPlayerTitlePlace)
			m_HdrPlayerTitlePlace.SetText(SM_PartyLoc.Text("#"));
		if (m_HdrPlayerTitleName)
			m_HdrPlayerTitleName.SetText(SM_PartyLoc.Text("#STR_SMP_00463"));
		if (m_HdrPlayerTitleTitle)
			m_HdrPlayerTitleTitle.SetText(SM_PartyLoc.Text("#STR_SMP_00457"));
		if (m_HdrPlayerTitleXP)
			m_HdrPlayerTitleXP.SetText(SM_PartyLoc.Text("#STR_SMP_00977"));
		if (m_HdrPlayerTitleClan)
			m_HdrPlayerTitleClan.SetText(SM_PartyLoc.Text("#STR_SMP_00505"));
		if (m_PlayerTitlesRefreshBtn)
			m_PlayerTitlesRefreshBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00715"));
		if (m_PlayerTitlesHintText)
			m_PlayerTitlesHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00473"));

		SetTabText(9, m_TabStorageBtn, "#STR_SMP_00919");
		m_HdrStorageItem.SetText(SM_PartyLoc.Text("#STR_SMP_00827"));
		m_HdrStorageQty.SetText(SM_PartyLoc.Text("#STR_SMP_00569"));
		m_HdrStorageWho.SetText(SM_PartyLoc.Text("#STR_SMP_00590"));
		m_HdrStorageDate.SetText(SM_PartyLoc.Text("#STR_SMP_00398"));
		m_DepositToStorageBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00806"));
		m_TakeFromStorageBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00321"));
		if (m_StorageSearchHint)
			m_StorageSearchHint.SetText(SM_PartyLoc.Text("#STR_SMP_00793"));

		m_DescTitleText.SetText(SM_PartyLoc.Text("#STR_SMP_00731"));
		m_SaveDescBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00967"));
		m_CloseDescBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00662"));

		m_PermsHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_00823"));
		m_PermsHdrAction.SetText(SM_PartyLoc.Text("#STR_SMP_00401"));
		m_PermsHdrRank.SetText(SM_PartyLoc.Text("#STR_SMP_00645"));
		m_PermRankUpBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00790"));
		m_PermRankDownBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00811"));
		m_PermsCloseBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00662"));
		m_WebhookSaveBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00967"));
		m_WebhookClearBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00764"));
		m_WebhookTestBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00992"));
		m_WebhookAuditBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00323"));

		m_InfoDescLabel.SetText(SM_PartyLoc.Text("#STR_SMP_00735"));
		m_InfoMembersLabel.SetText(SM_PartyLoc.Text("#STR_SMP_00966"));
		m_InfoHdrName.SetText(SM_PartyLoc.Text("#STR_SMP_00477"));
		m_InfoHdrRank.SetText(SM_PartyLoc.Text("#STR_SMP_00882"));
		if (m_InfoHdrTitle)
			m_InfoHdrTitle.SetText(SM_PartyLoc.Text("#STR_SMP_00457"));
		m_InfoHdrStatus.SetText(SM_PartyLoc.Text("#STR_SMP_00977"));
		m_ApplyBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00791"));
		m_AdminEnterClanBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00338"));
		m_CloseInfoBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00441"));
	}

	override void OnShow()
	{
		super.OnShow();
		SM_VPPInputSuppressor.SuppressVPPAdminInputs();
		SuppressMouseLookInput();

		Mission mission = GetGame().GetMission();
		if (mission)
			mission.PlayerControlDisable(INPUT_EXCLUDE_ALL);

		SM_ClanClientData.OnStateChanged.Insert(this.RefreshAll);
		SM_ClanClientData.OnClanListChanged.Insert(this.RefreshClans);
		SM_ClanClientData.OnPlayersChanged.Insert(this.RefreshPlayers);
		SM_ClanClientData.OnClanInfoChanged.Insert(this.OnClanInfoArrived);
		SM_ClanClientData.OnTopsChanged.Insert(this.OnTopsArrived);
		SM_ClanClientData.OnMarketChanged.Insert(this.RefreshMarket);
		SM_ClanClientData.OnServerMarketChanged.Insert(this.HandleServerMarketChanged);
		SM_ClanClientData.OnAuctionChanged.Insert(this.RefreshAuction);
		SM_ClanClientData.OnAchievementsChanged.Insert(this.RefreshAchievements);
		SM_ClanClientData.OnPlayerTitlesChanged.Insert(this.RefreshPlayerTitles);
		SM_ClanClientData.OnContractsChanged.Insert(this.OnContractsChanged);
		SM_ClanClientData.OnContractTargetPreviewChanged.Insert(this.OnContractTargetPreviewChanged);
		SM_ClanClientData.OnAdminChatLogChanged.Insert(this.RefreshAdminChatLog);
		SM_ClanClientData.OnOnlinePlayersChanged.Insert(this.RefreshOnlineWidget);
		SM_ClanClientData.OnStorageChanged.Insert(this.RefreshStorage);

		SendSimpleRpc(SM_PartyRPC.REQUEST_STATE);
		SendSimpleRpc(SM_PartyRPC.REQUEST_CLAN_LIST);
		SendSimpleRpc(SM_PartyRPC.REQUEST_PLAYER_TITLES);
		RequestOnlinePlayers();
		RequestContracts();
		SM_ClanClientData.EnsurePersonalMapMarkersLoaded();

		m_InviteMode = false;
		m_DescMode = false;
		m_PermsMode = false;
		m_InfoMode = false;
		m_TopCategory = 0;
		m_MarketFilter = "";
		m_ServerMarketFilter = "";
		m_AuctionFilter = "";
		m_StorageFilter = "";
		m_AchievementScopeFilter = SM_AchievementScope.PERSONAL;
		m_ServerMarketFullRefreshRequested = false;
		m_OnlineRefreshTimer = 0;
		m_ContractTimer = 0;
		if (m_MarketSearchEdit)
			m_MarketSearchEdit.SetText(SM_PartyLoc.Text(""));
		if (m_ServerMarketSearchEdit)
			m_ServerMarketSearchEdit.SetText(SM_PartyLoc.Text(""));
		if (m_AuctionSearchEdit)
			m_AuctionSearchEdit.SetText(SM_PartyLoc.Text(""));
		if (m_AuctionStepEdit)
			m_AuctionStepEdit.SetText(SM_PartyLoc.Text(SM_ClanClientData.AuctionMinBidStep.ToString()));
		if (m_AuctionDurationEdit)
			m_AuctionDurationEdit.SetText(SM_PartyLoc.Text(SM_ClanClientData.AuctionDefaultDurationMinutes.ToString()));
		if (m_ContractPriceEdit)
			m_ContractPriceEdit.SetText(SM_PartyLoc.Text(SM_ClanClientData.ContractsMinPrice.ToString()));
		if (m_ContractDurationEdit)
			m_ContractDurationEdit.SetText(SM_PartyLoc.Text(SM_ClanClientData.ContractsMinDurationMinutes.ToString()));
		if (m_ContractQuantityEdit)
			m_ContractQuantityEdit.SetText(SM_PartyLoc.Text("1"));
		if (m_StorageSearchEdit)
			m_StorageSearchEdit.SetText(SM_PartyLoc.Text(""));
		HideDisbandConfirm();
		m_HasSelectedMapPosition = false;
		m_ShowAdminBaseMarkers = false;
		m_SelectedMarkerColor = SM_ClanClientData.ClanColorIndex;
		if (SM_ClanClientData.HasClan)
			m_MapMarkerMode = MAP_MARKER_MODE_CLAN;
		else
			m_MapMarkerMode = MAP_MARKER_MODE_PERSONAL;
		m_CurrentTab = 0;

		bool openMapOnShow = s_OpenMapOnShow;
		s_OpenMapOnShow = false;
		bool openMapFullscreen = s_OpenMapFullscreenOnShow;
		s_OpenMapFullscreenOnShow = false;

		if (openMapOnShow && SM_ClanClientData.MapEnabled)
			m_CurrentTab = 6;
		else if (!SM_ClanClientData.HasClan && SM_ClanClientData.Invites.Count() > 0)
			m_CurrentTab = 2;

		RefreshAll();

		// Хоткей карты открывает её сразу на весь экран. Делаем после RefreshAll,
		// т.к. ShowTab() внутри сбрасывает фуллскрин для не-картовых вкладок.
		if (openMapFullscreen && m_CurrentTab == 6)
			SetMapFullscreen(true);

		FocusMenuInput();
	}

	override void OnHide()
	{
		super.OnHide();

		ExitMapFullscreen();

		Mission mission = GetGame().GetMission();
		if (mission)
			mission.PlayerControlEnable(false);

		SM_ClanClientData.OnStateChanged.Remove(this.RefreshAll);
		SM_ClanClientData.OnClanListChanged.Remove(this.RefreshClans);
		SM_ClanClientData.OnPlayersChanged.Remove(this.RefreshPlayers);
		SM_ClanClientData.OnClanInfoChanged.Remove(this.OnClanInfoArrived);
		SM_ClanClientData.OnTopsChanged.Remove(this.OnTopsArrived);
		SM_ClanClientData.OnMarketChanged.Remove(this.RefreshMarket);
		SM_ClanClientData.OnServerMarketChanged.Remove(this.HandleServerMarketChanged);
		SM_ClanClientData.OnAuctionChanged.Remove(this.RefreshAuction);
		SM_ClanClientData.OnAchievementsChanged.Remove(this.RefreshAchievements);
		SM_ClanClientData.OnPlayerTitlesChanged.Remove(this.RefreshPlayerTitles);
		SM_ClanClientData.OnContractsChanged.Remove(this.OnContractsChanged);
		SM_ClanClientData.OnContractTargetPreviewChanged.Remove(this.OnContractTargetPreviewChanged);
		SM_ClanClientData.OnAdminChatLogChanged.Remove(this.RefreshAdminChatLog);
		SM_ClanClientData.OnOnlinePlayersChanged.Remove(this.RefreshOnlineWidget);
		SM_ClanClientData.OnStorageChanged.Remove(this.RefreshStorage);

		HideOnlinePanel();
		ClearMarketPreview();
		ClearServerMarketPreview();
		ClearAuctionPreview();
		ClearStoragePreview();
		ClearContractTargetPreview();
		HideMapMarkerEditor();
		ClearMapMarkerWidgets();
		ClearItemTiles(m_MarketTileWidgets, m_MarketTileEntities);
		ClearItemTiles(m_ServerMarketTileWidgets, m_ServerMarketTileEntities);
		ClearItemTiles(m_AuctionTileWidgets, m_AuctionTileEntities);
		ClearItemTiles(m_StorageTileWidgets, m_StorageTileEntities);
	}

	override bool OnKeyPress(Widget w, int x, int y, int key)
	{
		SM_VPPInputSuppressor.SuppressVPPAdminInputs();

		if (key == KeyCode.KC_ESCAPE)
		{
			SM_CloseFromHotkey(true);
			return true;
		}

		if (key == KeyCode.KC_P)
		{
			if (SM_IsTextInputFocused())
				return super.OnKeyPress(w, x, y, key);

			SM_CloseFromHotkey(false);
			return true;
		}

		if (key == KeyCode.KC_M)
		{
			if (IsMapTabVisible())
			{
				if (SM_IsTextInputFocused())
					return super.OnKeyPress(w, x, y, key);

				SM_CloseFromHotkey(false);
				return true;
			}
		}

		return super.OnKeyPress(w, x, y, key);
	}

	override void Update(float timeslice)
	{
		SM_VPPInputSuppressor.SuppressVPPAdminInputs();
		SuppressMouseLookInput();
		super.Update(timeslice);

		if (IsClosing())
			return;

		UAInputAPI inputApi = GetUApi();
		UAInput backInput;
		if (inputApi)
			backInput = inputApi.GetInputByID(UAUIBack);

		if (backInput && backInput.LocalPress())
		{
			// Открытое подтверждение роспуска закрываем первым, не выходя из меню.
			SM_CloseFromHotkey(true);
			return;
		}

		UpdateCreateCounters();

		if (m_CurrentTab == 6)
		{
			UpdateMapCoordinates();
			UpdateMapMarkerWidgetPositions();
			m_MapRefreshTimer += timeslice;
			if (m_MapRefreshTimer >= 2.0)
			{
				m_MapRefreshTimer = 0;
				RefreshMapMarks();
			}
		}

		if (m_CurrentTab == 3)
		{
			m_TopsSeasonTimer += timeslice;
			if (m_TopsSeasonTimer >= 1.0)
			{
				m_TopsSeasonTimer = 0;
				if (SM_ClanClientData.TopSeasonSecondsLeft > 0)
					SM_ClanClientData.TopSeasonSecondsLeft--;
				UpdateTopsHeaderAndHint();
			}
		}

		if (m_CurrentTab == 8)
		{
			m_ServerMarketRefreshTimer += timeslice;
			if (m_ServerMarketRefreshTimer >= 1.0)
			{
				m_ServerMarketRefreshTimer = 0;
				if (SM_ClanClientData.ServerMarketNextRefreshSeconds > 0)
					SM_ClanClientData.ServerMarketNextRefreshSeconds--;
				if (SM_ClanClientData.ServerMarketNextRefreshSeconds <= 0)
					RequestServerMarketFullRefresh();
				UpdateServerMarketTimerView();
			}
		}

		if (m_CurrentTab == 10)
		{
			m_AuctionTimer += timeslice;
			if (m_AuctionTimer >= 1.0)
			{
				m_AuctionTimer = 0;
				for (int a = 0; a < SM_ClanClientData.AuctionLots.Count(); a++)
				{
					if (SM_ClanClientData.AuctionLots[a].SecondsLeft > 0)
						SM_ClanClientData.AuctionLots[a].SecondsLeft--;
				}
				RefreshAuction();
			}
		}

		if (m_CurrentTab == 14)
		{
			m_ContractTimer += timeslice;
			if (m_ContractTimer >= 1.0)
			{
				m_ContractTimer = 0;
				TickContractTimers();
			}
		}
		else
		{
			m_ContractTimer = 0;
		}

		m_OnlineRefreshTimer += timeslice;
		if (m_OnlineRefreshTimer >= 10.0)
		{
			m_OnlineRefreshTimer = 0;
			RequestOnlinePlayers();
		}

		m_MoneyRefreshTimer += timeslice;
		if (m_MoneyRefreshTimer >= 1.0)
		{
			m_MoneyRefreshTimer = 0;
			RefreshMoney();
		}
	}

	protected void SetCreateCounter(TextWidget widget, int current, int maxValue)
	{
		if (!widget)
			return;

		widget.SetText(SM_PartyLoc.Text(current.ToString() + "/" + maxValue.ToString()));
		if (current > maxValue)
			widget.SetColor(ARGB(255, 235, 80, 80));
		else
			widget.SetColor(ARGB(255, 201, 184, 160));
	}

	protected bool IsCreateFormValid(string name, string tag, string desc)
	{
		name = name.Trim();
		tag = tag.Trim();
		desc = desc.Trim();

		int nameLength = name.LengthUtf8();
		if (nameLength < SM_ClanClientData.MinClanNameLength)
			return false;
		if (nameLength > SM_ClanClientData.MaxClanNameLength)
			return false;
		if (tag.LengthUtf8() > SM_ClanClientData.MaxClanTagLength)
			return false;
		if (tag != "" && !SM_PartyUtil.IsLatinAlnum(tag))
			return false;
		if (desc.LengthUtf8() > SM_ClanClientData.MaxClanDescriptionLength)
			return false;
		return true;
	}

	protected void GetCreateFormValues(out string name, out string tag, out string desc)
	{
		name = "";
		tag = "";
		desc = "";

		if (m_ClanNameEdit)
			name = m_ClanNameEdit.GetText();
		if (m_ClanTagEdit)
			tag = m_ClanTagEdit.GetText();
		if (m_ClanDescEdit)
			m_ClanDescEdit.GetText(desc);
	}

	protected bool IsCurrentCreateFormValid()
	{
		string name;
		string tag;
		string desc;
		GetCreateFormValues(name, tag, desc);
		return IsCreateFormValid(name, tag, desc);
	}

	protected void SetCreateButtonHover()
	{
		bool canCreate = IsCurrentCreateFormValid();

		if (m_CreateClanBtnBg)
		{
			if (canCreate)
				m_CreateClanBtnBg.SetColor(ARGB(235, 70, 60, 50));
			else
				m_CreateClanBtnBg.SetColor(ARGB(235, 62, 53, 44));
		}

		if (m_CreateClanBtn)
			m_CreateClanBtn.SetTextColor(ARGB(255, 255, 255, 242));
	}

	protected void UpdateCreateButtonState(string name, string tag, string desc)
	{
		bool canCreate = IsCreateFormValid(name, tag, desc);

		if (m_CreateClanBtn)
		{
			m_CreateClanBtn.Enable(true);
			m_CreateClanBtn.SetTextColor(ARGB(255, 255, 255, 242));
		}

		if (m_CreateClanBtnBg)
		{
			if (canCreate)
				m_CreateClanBtnBg.SetColor(ARGB(235, 53, 45, 38));
			else
				m_CreateClanBtnBg.SetColor(ARGB(235, 50, 43, 35));
		}
	}

	protected void UpdateCreateCounters()
	{
		string name;
		string tag;
		string desc;
		GetCreateFormValues(name, tag, desc);

		SetCreateCounter(m_CreateNameCounterText, name.LengthUtf8(), SM_ClanClientData.MaxClanNameLength);
		SetCreateCounter(m_CreateTagCounterText, tag.LengthUtf8(), SM_ClanClientData.MaxClanTagLength);
		SetCreateCounter(m_CreateDescCounterText, desc.LengthUtf8(), SM_ClanClientData.MaxClanDescriptionLength);
		UpdateCreateButtonState(name, tag, desc);
	}

	protected void ApplyCreateLogo()
	{
		if (!m_CreateClanLogoImage)
			return;

		string logoPath = SM_ClanClientData.CreateClanLogoPaa;
		logoPath = logoPath.Trim();
		if (logoPath == "")
			logoPath = "SM_PartyMod\\GUI\\logo\\logo.paa";
		if (logoPath == "SM_PartyMod\\GUI\\pings\\ping.paa")
			logoPath = "SM_PartyMod\\GUI\\logo\\logo.paa";

		if (logoPath == m_AppliedCreateLogoPaa)
			return;

		m_AppliedCreateLogoPaa = logoPath;
		m_CreateClanLogoImage.LoadImageFile(0, logoPath);
		m_CreateClanLogoImage.SetImage(0);
	}

	protected void RefreshMoney()
	{
		if (!m_MoneyText)
			return;

		if (SM_ClanClientData.CurrencyItems.Count() == 0)
		{
			m_MoneyText.Show(false);
			if (m_MoneyValueText)
				m_MoneyValueText.Show(false);
			return;
		}

		m_MoneyText.Show(true);
		if (m_MoneyValueText)
		{
			m_MoneyText.SetText(SM_PartyLoc.Text("#STR_SMP_00406"));
			m_MoneyValueText.Show(true);
			m_MoneyValueText.SetText(SM_PartyLoc.Text(GetMyCurrencyPoints().ToString()));
		}
		else
		{
			m_MoneyText.SetText(SM_PartyLoc.Text("#STR_SMP_00407" + GetMyCurrencyPoints().ToString()));
		}
	}


	protected void CenterMapOnPlayer()
	{
		if (!m_ClanMap)
		{
			Print(SM_PartyLoc.Text("#STR_SMP_00171"));
			if (m_MapHeaderText)
				m_MapHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_00501"));
			return;
		}

		float scale = 0.33;
		vector mapPos;

		Man player = GetGame().GetPlayer();
		if (player)
		{
			mapPos = player.GetPosition();
		}
		else
		{
			mapPos = GetGame().ConfigGetVector(string.Format("CfgWorlds %1 centerPosition", GetGame().GetWorldName()));
		}

		if (layoutRoot)
			layoutRoot.Update();
		if (m_ActiveMapHolder)
			m_ActiveMapHolder.Update();
		m_ClanMap.Update();
		m_ClanMap.SetScale(scale);
		m_ClanMap.SetMapPos(mapPos);
	}

	protected void ShowLocalPlayerOnMap()
	{
		if (!m_ClanMap)
			return;

		Man player = GetGame().GetPlayer();
		if (!player)
			return;

		m_ClanMap.SetMapPos(player.GetPosition());
		m_ClanMap.Update();
		UpdateMapCoordinates();
		UpdateMapMarkerWidgetPositions();
	}

	protected void RefreshMapView()
	{
		if (!m_ClanMap)
			return;

		m_ClanMap.Show(true);
		if (layoutRoot)
			layoutRoot.Update();
		if (m_ActiveMapHolder)
			m_ActiveMapHolder.Update();
		m_ClanMap.Update();

		CenterMapOnPlayer();
		RefreshMapMarks();
		RefreshMapMarkerList();

		if (m_MapFullscreen)
			DockSidebarRight();
		else
			RestoreSidebarPosition();
	}

	protected bool IsScreenPointInWidget(Widget widget, int x, int y)
	{
		if (!widget)
			return false;
		if (!widget.IsVisible())
			return false;

		float wx;
		float wy;
		float ww;
		float wh;
		widget.GetScreenPos(wx, wy);
		widget.GetScreenSize(ww, wh);

		if (x < wx)
			return false;
		if (y < wy)
			return false;
		if (x > wx + ww)
			return false;
		if (y > wy + wh)
			return false;
		return true;
	}

	protected string FormatMapCoordinates(vector mapPos)
	{
		int coordX = Math.Round(mapPos[0]);
		int coordZ = Math.Round(mapPos[2]);
		int coordY = Math.Round(GetGame().SurfaceY(mapPos[0], mapPos[2]));
		return "#STR_SMP_00585" + coordX.ToString() + "  Z " + coordZ.ToString() + "  Y " + coordY.ToString();
	}

	protected void UpdateMapCoordinates()
	{
		if (!m_MapCoordsText)
			return;
		if (!m_ClanMap)
		{
			m_MapCoordsText.Show(false);
			return;
		}
		if (!m_PanelMap || !m_PanelMap.IsVisible())
		{
			m_MapCoordsText.Show(false);
			return;
		}

		int mouseX;
		int mouseY;
		GetMousePos(mouseX, mouseY);

		if (!IsScreenPointInWidget(m_ClanMap, mouseX, mouseY))
		{
			m_MapCoordsText.Show(false);
			return;
		}
		if (IsScreenPointInWidget(m_MapMarkersBg, mouseX, mouseY))
		{
			m_MapCoordsText.Show(false);
			return;
		}

		vector mapPos = m_ClanMap.ScreenToMap(Vector(mouseX, mouseY, 0));
		m_MapCoordsText.SetText(SM_PartyLoc.Text(FormatMapCoordinates(mapPos)));
		m_MapCoordsText.Show(true);
	}

	protected bool IsMapMarkerModeAvailable(int mode)
	{
		if (mode == MAP_MARKER_MODE_PERSONAL)
			return true;
		if (mode == MAP_MARKER_MODE_CLAN)
			return SM_ClanClientData.HasClan;
		if (mode == MAP_MARKER_MODE_SERVER)
			return SM_ClanClientData.ServerMapMarkers.Count() > 0;
		if (mode == MAP_MARKER_MODE_ALL)
			return true;
		return false;
	}

	// Список доступных режимов в каноническом порядке для переключения < >.
	protected void GetAvailableMapMarkerModes(array<int> modes)
	{
		modes.Clear();
		modes.Insert(MAP_MARKER_MODE_PERSONAL);
		if (IsMapMarkerModeAvailable(MAP_MARKER_MODE_CLAN))
			modes.Insert(MAP_MARKER_MODE_CLAN);
		if (IsMapMarkerModeAvailable(MAP_MARKER_MODE_SERVER))
			modes.Insert(MAP_MARKER_MODE_SERVER);
		modes.Insert(MAP_MARKER_MODE_ALL);
	}

	protected void NormalizeMapMarkerMode()
	{
		if (IsMapMarkerModeAvailable(m_MapMarkerMode))
			return;

		if (SM_ClanClientData.HasClan)
			m_MapMarkerMode = MAP_MARKER_MODE_CLAN;
		else
			m_MapMarkerMode = MAP_MARKER_MODE_PERSONAL;

		if (!IsMapMarkerModeAvailable(m_MapMarkerMode))
			m_MapMarkerMode = MAP_MARKER_MODE_PERSONAL;
	}

	// dir = +1 (следующий, >) или -1 (предыдущий, <)
	protected void CycleMapMarkerMode(int dir)
	{
		NormalizeMapMarkerMode();

		array<int> modes = new array<int>;
		GetAvailableMapMarkerModes(modes);
		if (modes.Count() == 0)
			return;

		int idx = modes.Find(m_MapMarkerMode);
		if (idx < 0)
			idx = 0;

		idx = (idx + dir + modes.Count()) % modes.Count();
		m_MapMarkerMode = modes[idx];

		m_HasSelectedMapPosition = false;
		HideMapMarkerEditor();
		RefreshMapMarks();
		RefreshMapMarkerList();
	}

	protected string GetMapMarkerModeText()
	{
		if (m_MapMarkerMode == MAP_MARKER_MODE_CLAN)
			return "#STR_SMP_00563";
		if (m_MapMarkerMode == MAP_MARKER_MODE_SERVER)
			return "#STR_SMP_00911";
		if (m_MapMarkerMode == MAP_MARKER_MODE_ALL)
			return "#STR_SMP_00719";
		return "#STR_SMP_00611";
	}

	// Можно ли добавлять метки в текущем режиме (Серверные и Общие — только просмотр).
	protected bool IsMapMarkerModeEditable()
	{
		return m_MapMarkerMode == MAP_MARKER_MODE_PERSONAL || m_MapMarkerMode == MAP_MARKER_MODE_CLAN;
	}

	protected string GetMapMarkerAddText()
	{
		if (m_MapMarkerMode == MAP_MARKER_MODE_CLAN)
			return "#STR_SMP_00415";
		return "#STR_SMP_00416";
	}

	protected bool IsMapMarkerTypeEditable(int markerType)
	{
		if (markerType == MAP_MARKER_ROW_PERSONAL)
			return true;
		if (markerType == MAP_MARKER_ROW_CLAN && SM_ClanClientData.HasClan)
			return true;
		return false;
	}

	protected string GetMapMarkerTypeName(int markerType)
	{
		if (markerType == MAP_MARKER_ROW_PERSONAL)
			return "#STR_SMP_00610";
		if (markerType == MAP_MARKER_ROW_CLAN)
			return "#STR_SMP_00562";
		if (markerType == MAP_MARKER_ROW_SERVER)
			return "#STR_SMP_00910";
		if (markerType == MAP_MARKER_ROW_ADMIN_BASE)
			return "#STR_SMP_00263";
		return "#STR_SMP_00639";
	}

	protected void ClearMapMarkerWidgets()
	{
		ClearBaseRadiusWidgets();

		for (int i = 0; i < m_MapMarkerWidgetRoots.Count(); i++)
		{
			Widget root = m_MapMarkerWidgetRoots[i];
			if (root)
				root.Unlink();
		}

		m_MapMarkerWidgetRoots.Clear();
		m_MapMarkerWidgetButtons.Clear();
		m_MapMarkerWidgetIcons.Clear();
		m_MapMarkerWidgetAccents.Clear();
		m_MapMarkerWidgetTypes.Clear();
		m_MapMarkerWidgetIds.Clear();
		m_MapMarkerWidgetPositions.Clear();
		m_MapMarkerWidgetColors.Clear();
		m_MapMarkerWidgetIconIdx.Clear();
		m_MapMarkerWidgetNames.Clear();
	}

	protected void ClearBaseRadiusWidgets()
	{
		for (int i = 0; i < m_MapBaseRadiusSegments.Count(); i++)
		{
			Widget segment = m_MapBaseRadiusSegments[i];
			if (segment)
				segment.Unlink();
		}

		m_MapBaseRadiusSegments.Clear();
	}

	protected int GetBaseRadiusLineColor()
	{
		int color = SM_ClanClientData.GetClanColor();
		color = SM_ClanColors.ResolveColor(color);
		int red = (color >> 16) & 0xFF;
		int green = (color >> 8) & 0xFF;
		int blue = color & 0xFF;
		return ARGB(255, red, green, blue);
	}

	protected float GetBaseRadiusMeters()
	{
		float radius = SM_ClanClientData.BaseRadius;
		if (radius <= 0)
			radius = 150;
		return radius;
	}

	protected bool IsBaseRadiusEnabled()
	{
		if (!SM_ClanClientData.BaseRadiusEnabled)
			return false;
		if (!SM_ClanClientData.HasClan)
			return false;
		if (!SM_ClanClientData.HasBase)
			return false;
		if (GetBaseRadiusMeters() <= 0)
			return false;
		return true;
	}

	protected void RebuildBaseRadiusWidgets()
	{
		ClearBaseRadiusWidgets();
		if (!m_MapMarkerWidgetLayer)
			return;
		if (!IsBaseRadiusEnabled())
			return;

		int lineColor = GetBaseRadiusLineColor();
		for (int i = 0; i < BASE_RADIUS_SEGMENTS; i++)
		{
			Widget segment = GetGame().GetWorkspace().CreateWidgets("SM_PartyMod/GUI/layouts/SM_MapCircleSegment.layout", m_MapMarkerWidgetLayer);
			if (!segment)
				continue;
			segment.SetColor(lineColor);
			segment.Show(false);
			m_MapBaseRadiusSegments.Insert(segment);
		}
	}

	protected vector GetBaseRadiusWorldPoint(float angle)
	{
		float radius = GetBaseRadiusMeters();
		vector basePos = SM_ClanClientData.BasePos;
		float x = basePos[0] + Math.Cos(angle) * radius;
		float z = basePos[2] + Math.Sin(angle) * radius;
		return Vector(x, basePos[1], z);
	}

	protected string GetBaseRadiusMapIcon()
	{
		return "\\dz\\gear\\navigation\\data\\map_border_cross_ca.paa";
	}

	protected void AddBaseRadiusMapMarks()
	{
		if (!m_ClanMap)
			return;
		if (!IsBaseRadiusEnabled())
			return;

		int color = GetBaseRadiusLineColor();
		string icon = GetBaseRadiusMapIcon();
		for (int i = 0; i < BASE_RADIUS_MAP_POINTS; i++)
		{
			float angle = (Math.PI2 * i) / BASE_RADIUS_MAP_POINTS;
			m_ClanMap.AddUserMark(GetBaseRadiusWorldPoint(angle), "", color, icon);
		}
	}

	protected bool SegmentIntersectsMap(float x1, float y1, float x2, float y2, float mapX, float mapY, float mapW, float mapH)
	{
		float minX = x1;
		float maxX = x1;
		float minY = y1;
		float maxY = y1;
		if (x2 < minX)
			minX = x2;
		if (x2 > maxX)
			maxX = x2;
		if (y2 < minY)
			minY = y2;
		if (y2 > maxY)
			maxY = y2;

		if (maxX < mapX)
			return false;
		if (minX > mapX + mapW)
			return false;
		if (maxY < mapY)
			return false;
		if (minY > mapY + mapH)
			return false;
		return true;
	}

	protected void UpdateBaseRadiusWidgetPositions(float layerX, float layerY, float mapX, float mapY, float mapW, float mapH)
	{
		if (!m_ClanMap)
			return;
		if (!IsBaseRadiusEnabled())
		{
			for (int h = 0; h < m_MapBaseRadiusSegments.Count(); h++)
			{
				if (m_MapBaseRadiusSegments[h])
					m_MapBaseRadiusSegments[h].Show(false);
			}
			return;
		}

		for (int i = 0; i < m_MapBaseRadiusSegments.Count(); i++)
		{
			Widget segment = m_MapBaseRadiusSegments[i];
			if (!segment)
				continue;

			float angleA = (Math.PI2 * i) / BASE_RADIUS_SEGMENTS;
			float angleB = (Math.PI2 * (i + 1)) / BASE_RADIUS_SEGMENTS;
			vector screenA = m_ClanMap.MapToScreen(GetBaseRadiusWorldPoint(angleA));
			vector screenB = m_ClanMap.MapToScreen(GetBaseRadiusWorldPoint(angleB));
			float x1 = screenA[0];
			float y1 = screenA[1];
			float x2 = screenB[0];
			float y2 = screenB[1];

			if (!SegmentIntersectsMap(x1, y1, x2, y2, mapX, mapY, mapW, mapH))
			{
				segment.Show(false);
				continue;
			}

			float dx = x2 - x1;
			float dy = y2 - y1;
			float length = vector.Distance(Vector(x1, y1, 0), Vector(x2, y2, 0));
			if (length < 1.0)
			{
				segment.Show(false);
				continue;
			}

			float midX = (x1 + x2) * 0.5;
			float midY = (y1 + y2) * 0.5;
			float angle = Math.Atan2(dy, dx) * Math.RAD2DEG;
			segment.SetSize(length, BASE_RADIUS_LINE_THICKNESS);
			segment.SetPos(midX - layerX - length * 0.5, midY - layerY - BASE_RADIUS_LINE_THICKNESS * 0.5);
			segment.SetRotation(0, 0, angle);
			segment.Show(true);
		}
	}

	protected void AddMapMarkerWidget(int markerType, int markerId, string name, vector position, int colorIndex, string iconPath, int iconIdx = 0)
	{
		if (!m_MapMarkerWidgetLayer)
			return;

		iconPath = SM_MapMarkerIconPath.ForImageWidget(iconPath);

		Widget root = GetGame().GetWorkspace().CreateWidgets("SM_PartyMod/GUI/layouts/SM_MapMarkerButton.layout", m_MapMarkerWidgetLayer);
		if (!root)
			return;

		ButtonWidget button = ButtonWidget.Cast(root.FindAnyWidget("MapMarkerWidgetButton"));
		ImageWidget icon = ImageWidget.Cast(root.FindAnyWidget("MapMarkerWidgetIcon"));
		Widget glow = root.FindAnyWidget("MapMarkerWidgetGlow");
		Widget accent = root.FindAnyWidget("MapMarkerWidgetAccent");
		int resolvedColor = SM_ClanColors.ResolveColor(colorIndex);

		if (button)
			button.SetFlags(WidgetFlags.NOFOCUS);
		if (glow)
			glow.Show(false);
		if (icon)
		{
			icon.LoadImageFile(0, iconPath);
			icon.SetColor(resolvedColor);
			icon.Show(false);
		}
		if (accent)
		{
			accent.SetColor(resolvedColor);
			accent.Show(false);
		}

		m_MapMarkerWidgetRoots.Insert(root);
		m_MapMarkerWidgetButtons.Insert(button);
		m_MapMarkerWidgetIcons.Insert(icon);
		m_MapMarkerWidgetAccents.Insert(accent);
		m_MapMarkerWidgetTypes.Insert(markerType);
		m_MapMarkerWidgetIds.Insert(markerId);
		m_MapMarkerWidgetPositions.Insert(position);
		m_MapMarkerWidgetColors.Insert(colorIndex);
		m_MapMarkerWidgetIconIdx.Insert(iconIdx);
		m_MapMarkerWidgetNames.Insert(name);
	}

	protected void RebuildMapMarkerWidgets()
	{
		ClearMapMarkerWidgets();

		if (!m_PanelMap || !m_PanelMap.IsVisible())
			return;
		if (!m_MapMarkerWidgetLayer)
			return;

		m_MapMarkerWidgetLayer.Show(true);
		NormalizeMapMarkerMode();
		SM_ClanClientData.EnsurePersonalMapMarkersLoaded();

		string clanMarkerIcon = SM_ClanMarkerIcon.GetPath();

		if (m_MapMarkerMode == MAP_MARKER_MODE_PERSONAL || m_MapMarkerMode == MAP_MARKER_MODE_ALL)
		{
			foreach (SM_PersonalMapMarker personalMarker : SM_ClanClientData.PersonalMapMarkers)
			{
				if (personalMarker)
					AddMapMarkerWidget(MAP_MARKER_ROW_PERSONAL, personalMarker.Id, personalMarker.Name, personalMarker.Position, personalMarker.Color, SM_MapMarkerIconSet.GetPath(personalMarker.Icon), personalMarker.Icon);
			}
		}

		if ((m_MapMarkerMode == MAP_MARKER_MODE_CLAN || m_MapMarkerMode == MAP_MARKER_MODE_ALL) && SM_ClanClientData.HasClan)
		{
			foreach (SM_ClanMapMarker clanMarker : SM_ClanClientData.MapMarkers)
			{
				if (clanMarker)
					AddMapMarkerWidget(MAP_MARKER_ROW_CLAN, clanMarker.Id, clanMarker.Name, clanMarker.Position, clanMarker.Color, SM_MapMarkerIconSet.GetPath(clanMarker.Icon), clanMarker.Icon);
			}
		}

		if (m_MapMarkerMode == MAP_MARKER_MODE_SERVER || m_MapMarkerMode == MAP_MARKER_MODE_ALL)
		{
			for (int s = 0; s < SM_ClanClientData.ServerMapMarkers.Count(); s++)
			{
				SM_ServerMapMarker serverMarker = SM_ClanClientData.ServerMapMarkers[s];
				if (!serverMarker)
					continue;

				string serverIcon = serverMarker.GetIconPath();

				AddMapMarkerWidget(MAP_MARKER_ROW_SERVER, s, serverMarker.Name, serverMarker.Position, serverMarker.GetColor(SM_ClanColors.GetColor(0)), serverIcon);
			}
		}

		if (m_ShowAdminBaseMarkers && SM_ClanClientData.IsAdmin)
		{
			for (int b = 0; b < SM_ClanClientData.AdminBaseMapMarkers.Count(); b++)
			{
				SM_AdminBaseMapMarker adminBaseMarker = SM_ClanClientData.AdminBaseMapMarkers[b];
				if (!adminBaseMarker)
					continue;

				string adminIcon = adminBaseMarker.GetIconPath();

				string adminLabel = adminBaseMarker.Label;
				if (adminLabel == "")
					adminLabel = "#STR_SMP_00266" + adminBaseMarker.ClanName + "#STR_SMP_00024" + adminBaseMarker.OwnerName;

				AddMapMarkerWidget(MAP_MARKER_ROW_ADMIN_BASE, b, adminLabel, adminBaseMarker.Position, adminBaseMarker.Color, adminIcon);
			}
		}

		if (m_HasSelectedMapPosition && IsMapMarkerModeEditable())
			AddMapMarkerWidget(m_MapMarkerMode, -1, "#STR_SMP_00708", m_SelectedMapPosition, m_SelectedMarkerColor, SM_MapMarkerIconSet.GetPath(m_SelectedMarkerIcon), m_SelectedMarkerIcon);

		UpdateMapMarkerWidgetPositions();
	}

	protected void UpdateMapMarkerWidgetPositions()
	{
		if (!m_ClanMap || !m_MapMarkerWidgetLayer)
			return;

		if (m_CurrentTab != 6 || !m_PanelMap || !m_PanelMap.IsVisible())
		{
			m_MapMarkerWidgetLayer.Show(false);
			return;
		}

		m_MapMarkerWidgetLayer.Show(true);

		float layerX;
		float layerY;
		m_MapMarkerWidgetLayer.GetScreenPos(layerX, layerY);

		float mapX;
		float mapY;
		float mapW;
		float mapH;
		m_ClanMap.GetScreenPos(mapX, mapY);
		m_ClanMap.GetScreenSize(mapW, mapH);

		for (int i = 0; i < m_MapMarkerWidgetRoots.Count(); i++)
		{
			Widget root = m_MapMarkerWidgetRoots[i];
			if (!root)
				continue;

			vector screenPos = m_ClanMap.MapToScreen(m_MapMarkerWidgetPositions[i]);
			float sx = screenPos[0];
			float sy = screenPos[1];

			if (sx < mapX || sy < mapY || sx > mapX + mapW || sy > mapY + mapH)
			{
				root.Show(false);
				continue;
			}

			root.SetPos(sx - layerX - 17, sy - layerY - 17);
			root.Show(true);
		}
	}

	protected int FindMapMarkerWidgetButtonIndex(Widget w)
	{
		for (int i = 0; i < m_MapMarkerWidgetButtons.Count(); i++)
		{
			if (m_MapMarkerWidgetButtons[i] && w == m_MapMarkerWidgetButtons[i])
				return i;
		}

		return -1;
	}

	protected void SelectMapMarkerRowByTypeAndId(int markerType, int markerId)
	{
		if (!m_MapMarkersList)
			return;

		for (int i = 0; i < m_MapMarkerRowIds.Count(); i++)
		{
			if (i >= m_MapMarkerRowTypes.Count())
				continue;

			if (m_MapMarkerRowTypes[i] == markerType && m_MapMarkerRowIds[i] == markerId)
			{
				m_MapMarkersList.SelectRow(i);
				Update3DToggleButton();
				return;
			}
		}
	}

	protected void HideMapMarkerEditor()
	{
		if (m_MapMarkerEditPanel)
			m_MapMarkerEditPanel.Show(false);
	}

	protected void SetMapMarkerEditorRgbValueTexts()
	{
		if (m_MapMarkerEditRedValueText)
			m_MapMarkerEditRedValueText.SetText(SM_PartyLoc.Text(SM_ClanColors.GetRed(m_MapMarkerEditColor).ToString()));
		if (m_MapMarkerEditGreenValueText)
			m_MapMarkerEditGreenValueText.SetText(SM_PartyLoc.Text(SM_ClanColors.GetGreen(m_MapMarkerEditColor).ToString()));
		if (m_MapMarkerEditBlueValueText)
			m_MapMarkerEditBlueValueText.SetText(SM_PartyLoc.Text(SM_ClanColors.GetBlue(m_MapMarkerEditColor).ToString()));
	}

	protected void SetMapMarkerEditorSlidersFromColor()
	{
		int red = SM_ClanColors.GetRed(m_MapMarkerEditColor);
		int green = SM_ClanColors.GetGreen(m_MapMarkerEditColor);
		int blue = SM_ClanColors.GetBlue(m_MapMarkerEditColor);

		if (m_MapMarkerEditRedSlider)
			m_MapMarkerEditRedSlider.SetCurrent(red);
		if (m_MapMarkerEditGreenSlider)
			m_MapMarkerEditGreenSlider.SetCurrent(green);
		if (m_MapMarkerEditBlueSlider)
			m_MapMarkerEditBlueSlider.SetCurrent(blue);

		SetMapMarkerEditorRgbValueTexts();
	}

	protected int GetMapMarkerEditorSliderValue(SliderWidget slider)
	{
		if (!slider)
			return 0;

		return SM_ClanColors.ClampChannel(Math.Round(slider.GetCurrent()));
	}

	protected void ApplyMapMarkerEditorRgbFromSliders(bool refreshPreview)
	{
		int red = GetMapMarkerEditorSliderValue(m_MapMarkerEditRedSlider);
		int green = GetMapMarkerEditorSliderValue(m_MapMarkerEditGreenSlider);
		int blue = GetMapMarkerEditorSliderValue(m_MapMarkerEditBlueSlider);
		m_MapMarkerEditColor = SM_ClanColors.FromRGB(red, green, blue);
		RefreshMapMarkerEditorColor();
		SetMapMarkerEditorRgbValueTexts();

		if (refreshPreview && m_MapMarkerEditNew)
		{
			m_SelectedMarkerColor = m_MapMarkerEditColor;
			RefreshMapMarks();
		}
	}

	protected void SetMapMarkerEditorRgbControlsEnabled(bool enabled)
	{
		if (m_MapMarkerEditRedEdit)
		{
			m_MapMarkerEditRedEdit.Enable(false);
			m_MapMarkerEditRedEdit.Show(false);
		}
		if (m_MapMarkerEditGreenEdit)
		{
			m_MapMarkerEditGreenEdit.Enable(false);
			m_MapMarkerEditGreenEdit.Show(false);
		}
		if (m_MapMarkerEditBlueEdit)
		{
			m_MapMarkerEditBlueEdit.Enable(false);
			m_MapMarkerEditBlueEdit.Show(false);
		}

		if (m_MapMarkerEditRedDecBtn)
			m_MapMarkerEditRedDecBtn.Show(false);
		if (m_MapMarkerEditRedIncBtn)
			m_MapMarkerEditRedIncBtn.Show(false);
		if (m_MapMarkerEditGreenDecBtn)
			m_MapMarkerEditGreenDecBtn.Show(false);
		if (m_MapMarkerEditGreenIncBtn)
			m_MapMarkerEditGreenIncBtn.Show(false);
		if (m_MapMarkerEditBlueDecBtn)
			m_MapMarkerEditBlueDecBtn.Show(false);
		if (m_MapMarkerEditBlueIncBtn)
			m_MapMarkerEditBlueIncBtn.Show(false);

		if (m_MapMarkerEditRedSlider)
		{
			m_MapMarkerEditRedSlider.Show(true);
			m_MapMarkerEditRedSlider.Enable(enabled);
		}
		if (m_MapMarkerEditGreenSlider)
		{
			m_MapMarkerEditGreenSlider.Show(true);
			m_MapMarkerEditGreenSlider.Enable(enabled);
		}
		if (m_MapMarkerEditBlueSlider)
		{
			m_MapMarkerEditBlueSlider.Show(true);
			m_MapMarkerEditBlueSlider.Enable(enabled);
		}

		if (m_MapMarkerEditRedValueText)
			m_MapMarkerEditRedValueText.Show(true);
		if (m_MapMarkerEditGreenValueText)
			m_MapMarkerEditGreenValueText.Show(true);
		if (m_MapMarkerEditBlueValueText)
			m_MapMarkerEditBlueValueText.Show(true);
	}

	protected void RefreshMapMarkerEditorColor()
	{
		m_MapMarkerEditColor = SM_ClanColors.NormalizeMarkerColor(m_MapMarkerEditColor);

		if (m_MapMarkerEditColorSwatch)
			m_MapMarkerEditColorSwatch.SetColor(SM_ClanColors.ResolveColor(m_MapMarkerEditColor));

		UpdateMapMarkerEditorIcon();
	}

	protected void UpdateMapMarkerEditorIcon()
	{
		m_MapMarkerEditIcon = SM_MapMarkerIconSet.Normalize(m_MapMarkerEditIcon);

		if (m_MapMarkerEditIconPreview)
		{
			m_MapMarkerEditIconPreview.LoadImageFile(0, SM_MapMarkerIconSet.GetPath(m_MapMarkerEditIcon));
			m_MapMarkerEditIconPreview.SetImage(0);
			m_MapMarkerEditIconPreview.SetColor(SM_ClanColors.ResolveColor(m_MapMarkerEditColor));
		}
		if (m_MapMarkerEditIconNameText)
			m_MapMarkerEditIconNameText.SetText(SM_PartyLoc.Text(SM_MapMarkerIconSet.GetNameKey(m_MapMarkerEditIcon)));
	}

	protected void ShowMapMarkerEditor(string name)
	{
		if (!m_MapMarkerEditPanel)
			return;

		bool editable = IsMapMarkerTypeEditable(m_MapMarkerEditType);
		if (m_MapMarkerEditNew && !IsMapMarkerModeEditable())
			editable = false;
		if (!editable)
		{
			HideMapMarkerEditor();
			return;
		}

		if (m_MapMarkerEditTitleText)
		{
			string title = GetMapMarkerTypeName(m_MapMarkerEditType);
			if (m_MapMarkerEditNew)
				title = "#STR_SMP_00708";
			m_MapMarkerEditTitleText.SetText(SM_PartyLoc.Text(title));
		}

		if (m_MapMarkerEditNameEdit)
		{
			m_MapMarkerEditNameEdit.SetText(SM_PartyLoc.Text(name));
			m_MapMarkerEditNameEdit.Enable(editable);
		}

		RefreshMapMarkerEditorColor();
		SetMapMarkerEditorSlidersFromColor();
		SetMapMarkerEditorRgbControlsEnabled(editable);

		if (m_MapMarkerEditInfoText)
		{
			if (editable)
				m_MapMarkerEditInfoText.SetText(SM_PartyLoc.Text("RGB 0-255"));
			else
				m_MapMarkerEditInfoText.SetText(SM_PartyLoc.Text("#STR_SMP_01000"));
		}

		if (m_MapMarkerEditColorPrevBtn)
			m_MapMarkerEditColorPrevBtn.Show(false);
		if (m_MapMarkerEditColorNextBtn)
			m_MapMarkerEditColorNextBtn.Show(false);
		Widget colorPrevBg = layoutRoot.FindAnyWidget("MapMarkerEditColorPrevBg");
		if (colorPrevBg)
			colorPrevBg.Show(false);
		Widget colorNextBg = layoutRoot.FindAnyWidget("MapMarkerEditColorNextBg");
		if (colorNextBg)
			colorNextBg.Show(false);

		if (m_MapMarkerEditIconPrevBtn)
		{
			m_MapMarkerEditIconPrevBtn.Show(editable);
			m_MapMarkerEditIconPrevBtn.Enable(editable);
		}
		if (m_MapMarkerEditIconNextBtn)
		{
			m_MapMarkerEditIconNextBtn.Show(editable);
			m_MapMarkerEditIconNextBtn.Enable(editable);
		}

		if (m_MapMarkerEditSaveBtn)
		{
			if (m_MapMarkerEditNew)
				SetButtonText(m_MapMarkerEditSaveBtn, "#STR_SMP_00962");
			else
				SetButtonText(m_MapMarkerEditSaveBtn, "#STR_SMP_00967");
			m_MapMarkerEditSaveBtn.Show(editable);
			ShowBtnDressing("MapMarkerEditSaveBtn", editable);
		}
		if (m_MapMarkerEditDeleteBtn)
		{
			m_MapMarkerEditDeleteBtn.Show(editable);
			ShowBtnDressing("MapMarkerEditDeleteBtn", editable);
		}

		m_MapMarkerEditPanel.Show(true);
	}

	protected void OpenMapMarkerEditorByWidgetIndex(int index)
	{
		if (index < 0 || index >= m_MapMarkerWidgetRoots.Count())
			return;
		if (index >= m_MapMarkerWidgetTypes.Count() || index >= m_MapMarkerWidgetIds.Count())
			return;
		if (index >= m_MapMarkerWidgetNames.Count() || index >= m_MapMarkerWidgetPositions.Count())
			return;
		if (index >= m_MapMarkerWidgetColors.Count())
			return;

		m_MapMarkerEditType = m_MapMarkerWidgetTypes[index];
		m_MapMarkerEditId = m_MapMarkerWidgetIds[index];
		m_MapMarkerEditPosition = m_MapMarkerWidgetPositions[index];
		m_MapMarkerEditColor = m_MapMarkerWidgetColors[index];
		if (index < m_MapMarkerWidgetIconIdx.Count())
			m_MapMarkerEditIcon = m_MapMarkerWidgetIconIdx[index];
		else
			m_MapMarkerEditIcon = 0;
		m_MapMarkerEditNew = m_MapMarkerEditId < 0;

		if (!m_MapMarkerEditNew)
			SelectMapMarkerRowByTypeAndId(m_MapMarkerEditType, m_MapMarkerEditId);

		if (m_MapSelectedText)
			m_MapSelectedText.SetText(SM_PartyLoc.Text(GetMapMarkerTypeName(m_MapMarkerEditType) + ": " + m_MapMarkerWidgetNames[index]));

		ShowMapMarkerEditor(m_MapMarkerWidgetNames[index]);
	}

	protected void OpenMapMarkerCreateEditor(vector position)
	{
		if (!IsMapMarkerModeEditable())
			return;

		string name = "#STR_SMP_00639";
		if (m_MapMarkerNameEdit)
		{
			name = m_MapMarkerNameEdit.GetText();
			name = name.Trim();
			if (name == "")
				name = "#STR_SMP_00639";
		}

		m_MapMarkerEditType = m_MapMarkerMode;
		m_MapMarkerEditId = -1;
		m_MapMarkerEditPosition = position;
		m_MapMarkerEditColor = m_SelectedMarkerColor;
		m_MapMarkerEditIcon = m_SelectedMarkerIcon;
		m_MapMarkerEditNew = true;

		ShowMapMarkerEditor(name);
	}

	protected void SendMapMarkerUpdateRpc(int markerId, vector position, string name, int color, int icon)
	{
		Man markerPlayer = GetGame().GetPlayer();
		if (!markerPlayer)
			return;

		ScriptRPC markerRpc = new ScriptRPC();
		markerRpc.Write(markerId);
		markerRpc.Write(position);
		markerRpc.Write(name);
		markerRpc.Write(color);
		markerRpc.Write(icon);
		markerRpc.Send(markerPlayer, SM_PartyRPC.MAP_MARKER_UPDATE, true, NULL);
	}

	protected void SaveMapMarkerEditor()
	{
		if (!IsMapMarkerTypeEditable(m_MapMarkerEditType))
			return;

		string name = "";
		if (m_MapMarkerEditNameEdit)
			name = m_MapMarkerEditNameEdit.GetText();
		name = name.Trim();
		if (name == "")
		{
			if (m_MapMarkerEditInfoText)
				m_MapMarkerEditInfoText.SetText(SM_PartyLoc.Text("#STR_SMP_00314"));
			return;
		}

		ApplyMapMarkerEditorRgbFromSliders(false);

		if (m_MapMarkerEditNew)
		{
			m_SelectedMapPosition = m_MapMarkerEditPosition;
			m_SelectedMarkerColor = m_MapMarkerEditColor;
			m_SelectedMarkerIcon = m_MapMarkerEditIcon;

			if (m_MapMarkerEditType == MAP_MARKER_ROW_PERSONAL)
			{
				string personalError;
				if (!SM_ClanClientData.AddPersonalMapMarker(name, m_MapMarkerEditPosition, m_MapMarkerEditColor, m_MapMarkerEditIcon, personalError))
				{
					if (m_MapMarkerEditInfoText)
						m_MapMarkerEditInfoText.SetText(SM_PartyLoc.Text(personalError));
					return;
				}
			}
			else if (m_MapMarkerEditType == MAP_MARKER_ROW_CLAN)
			{
				if (!SM_ClanClientData.HasClan)
					return;

				Man markerPlayer = GetGame().GetPlayer();
				if (markerPlayer)
				{
					ScriptRPC markerRpc = new ScriptRPC();
					markerRpc.Write(m_MapMarkerEditPosition);
					markerRpc.Write(name);
					markerRpc.Write(m_MapMarkerEditColor);
					markerRpc.Write(m_MapMarkerEditIcon);
					markerRpc.Send(markerPlayer, SM_PartyRPC.MAP_MARKER_ADD, true, NULL);
				}
			}

			m_HasSelectedMapPosition = false;
			if (m_MapMarkerNameEdit)
				m_MapMarkerNameEdit.SetText(SM_PartyLoc.Text(""));
		}
		else
		{
			if (m_MapMarkerEditType == MAP_MARKER_ROW_PERSONAL)
			{
				string updateError;
				if (!SM_ClanClientData.UpdatePersonalMapMarker(m_MapMarkerEditId, name, m_MapMarkerEditPosition, m_MapMarkerEditColor, m_MapMarkerEditIcon, updateError))
				{
					if (m_MapMarkerEditInfoText)
						m_MapMarkerEditInfoText.SetText(SM_PartyLoc.Text(updateError));
					return;
				}
			}
			else if (m_MapMarkerEditType == MAP_MARKER_ROW_CLAN)
			{
				SendMapMarkerUpdateRpc(m_MapMarkerEditId, m_MapMarkerEditPosition, name, m_MapMarkerEditColor, m_MapMarkerEditIcon);
				foreach (SM_ClanMapMarker clanMarker : SM_ClanClientData.MapMarkers)
				{
					if (!clanMarker || clanMarker.Id != m_MapMarkerEditId)
						continue;

					clanMarker.Name = name;
					clanMarker.Position = m_MapMarkerEditPosition;
					clanMarker.Color = m_MapMarkerEditColor;
					clanMarker.Icon = m_MapMarkerEditIcon;
					SM_ClanClientData.MapMarkers3DDirty = true;
					break;
				}
			}
		}

		HideMapMarkerEditor();
		RefreshMapMarks();
		RefreshMapMarkerList();
	}

	protected void DeleteMapMarkerEditor()
	{
		if (m_MapMarkerEditNew)
		{
			m_HasSelectedMapPosition = false;
			HideMapMarkerEditor();
			RefreshMapMarks();
			RefreshMapMarkerList();
			return;
		}

		if (m_MapMarkerEditType == MAP_MARKER_ROW_PERSONAL)
		{
			SM_ClanClientData.RemovePersonalMapMarker(m_MapMarkerEditId);
			HideMapMarkerEditor();
			RefreshMapMarks();
			RefreshMapMarkerList();
		}
		else if (m_MapMarkerEditType == MAP_MARKER_ROW_CLAN)
		{
			if (SM_ClanClientData.HasClan)
				SendIntRpc(SM_PartyRPC.MAP_MARKER_REMOVE, m_MapMarkerEditId);
			HideMapMarkerEditor();
		}
	}

	protected void RefreshMapMarks()
	{
		if (!m_ClanMap)
		{
			ClearMapMarkerWidgets();
			return;
		}

		m_ClanMap.ClearUserMarks();

		Man player = GetGame().GetPlayer();
		string localPlayerUid = "";
		PlayerBase localPlayer = PlayerBase.Cast(player);
		if (localPlayer && localPlayer.GetIdentity())
			localPlayerUid = localPlayer.GetIdentity().GetPlainId();
		string localPlayerIcon = SM_PlayerMarkerIcon.GetPath();
		if (localPlayer && localPlayer.IsInVehicle())
			localPlayerIcon = SM_MapMarkerIconSet.GetPath(6);
		if (player)
			m_ClanMap.AddUserMark(player.GetPosition(), SM_PartyLoc.Text("#STR_SMP_00344"), ARGB(255, 255, 255, 242), localPlayerIcon);

		NormalizeMapMarkerMode();
		SM_ClanClientData.EnsurePersonalMapMarkersLoaded();
		string clanMarkerIcon = SM_ClanMarkerIcon.GetPath();

		if (m_MapMarkerMode == MAP_MARKER_MODE_PERSONAL || m_MapMarkerMode == MAP_MARKER_MODE_ALL)
		{
			foreach (SM_PersonalMapMarker personalMarker : SM_ClanClientData.PersonalMapMarkers)
			{
				if (personalMarker)
					m_ClanMap.AddUserMark(personalMarker.Position, SM_PartyLoc.Text(personalMarker.Name), SM_ClanColors.ResolveColor(personalMarker.Color), SM_MapMarkerIconSet.GetPath(personalMarker.Icon));
			}
		}

		if (m_MapMarkerMode == MAP_MARKER_MODE_SERVER || m_MapMarkerMode == MAP_MARKER_MODE_ALL)
		{
			foreach (SM_ServerMapMarker serverMarker : SM_ClanClientData.ServerMapMarkers)
			{
				if (!serverMarker)
					continue;

				string serverIcon = serverMarker.GetMapIconPath();

				m_ClanMap.AddUserMark(serverMarker.Position, SM_PartyLoc.Text(serverMarker.Name), SM_ClanColors.ResolveColor(serverMarker.GetColor(SM_ClanColors.GetColor(0))), serverIcon);
			}
		}

		if (m_ShowAdminBaseMarkers && SM_ClanClientData.IsAdmin)
		{
			foreach (SM_AdminBaseMapMarker adminBaseMarker : SM_ClanClientData.AdminBaseMapMarkers)
			{
				if (!adminBaseMarker)
					continue;

				string adminBaseIcon = adminBaseMarker.GetMapIconPath();

				string adminBaseLabel = adminBaseMarker.Label;
				if (adminBaseLabel == "")
					adminBaseLabel = "#STR_SMP_00266" + adminBaseMarker.ClanName + "#STR_SMP_00024" + adminBaseMarker.OwnerName;

				m_ClanMap.AddUserMark(adminBaseMarker.Position, SM_PartyLoc.Text(adminBaseLabel), SM_ClanColors.ResolveColor(adminBaseMarker.Color), adminBaseIcon);
			}
		}

		if (SM_ClanClientData.HasClan)
		{
			int clanColor = SM_ClanClientData.GetClanColor();
			foreach (SM_ClanMemberView member : SM_ClanClientData.HudMembers)
			{
				if (!member)
					continue;
				if (member.Uid != "" && member.Uid == localPlayerUid)
					continue;
				string memberIcon = SM_PlayerMarkerIcon.GetPath();
				if (member.Status & SM_MemberStatus.IN_VEHICLE)
					memberIcon = SM_MapMarkerIconSet.GetPath(6);
				m_ClanMap.AddUserMark(member.Position, SM_PartyLoc.Text(member.Name), clanColor, memberIcon);
			}

			if (m_MapMarkerMode == MAP_MARKER_MODE_CLAN || m_MapMarkerMode == MAP_MARKER_MODE_ALL)
			{
				foreach (SM_ClanMapMarker marker : SM_ClanClientData.MapMarkers)
				{
					m_ClanMap.AddUserMark(marker.Position, SM_PartyLoc.Text(marker.Name), SM_ClanColors.ResolveColor(marker.Color), SM_MapMarkerIconSet.GetPath(marker.Icon));
				}
			}

			if (SM_ClanClientData.HasBase)
			{
				AddBaseRadiusMapMarks();
				m_ClanMap.AddUserMark(SM_ClanClientData.BasePos, SM_PartyLoc.Text("#STR_SMP_00308"), ARGB(255, 199, 124, 55), "\\dz\\gear\\navigation\\data\\map_camp_ca.paa");
			}

			foreach (SM_ClanPing ping : SM_ClanClientData.Pings)
			{
				if (!ping)
					continue;

				m_ClanMap.AddUserMark(ping.Position, SM_PartyLoc.Text(ping.GetMapLabel()), ping.GetColor(ARGB(255, 199, 124, 55)), ping.GetIconPath());
			}
		}

		foreach (SM_ContractMarkerView contractMarker : SM_ClanClientData.ContractMarkers)
		{
			if (!contractMarker)
				continue;
			string contractLabel = "#STR_SMP_00582" + contractMarker.Label;
			m_ClanMap.AddUserMark(contractMarker.Position, SM_PartyLoc.Text(contractLabel), GetContractMapMarkerColor(contractMarker.Type), GetContractMapMarkerIcon(contractMarker.Type));
		}

		if (m_HasSelectedMapPosition && IsMapMarkerModeEditable())
			m_ClanMap.AddUserMark(m_SelectedMapPosition, SM_PartyLoc.Text("#STR_SMP_00708"), SM_ClanColors.ResolveColor(m_SelectedMarkerColor), SM_MapMarkerIconSet.GetPath(m_SelectedMarkerIcon));

		RebuildMapMarkerWidgets();
	}

	protected void RefreshMapControls()
	{
		bool hasClan = SM_ClanClientData.HasClan;
		NormalizeMapMarkerMode();
		bool showAdminBasesButton = SM_ClanClientData.IsAdmin && SM_ClanClientData.AdminBaseMapMarkers.Count() > 0;
		if (!showAdminBasesButton)
			m_ShowAdminBaseMarkers = false;

		if (m_MapHeaderText)
		{
			if (hasClan)
				m_MapHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_00500"));
			else
				m_MapHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_00498"));
		}

		if (m_MapHintText)
		{
			if (m_MapMarkerMode == MAP_MARKER_MODE_SERVER)
				m_MapHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00912"));
			else if (m_MapMarkerMode == MAP_MARKER_MODE_ALL)
				m_MapHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00720"));
			else
				m_MapHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00224"));
		}

		if (m_MapModeLabel)
			m_MapModeLabel.SetText(SM_PartyLoc.Text(GetMapMarkerModeText()));
		if (m_MapMarkersList)
			m_MapMarkersList.Show(true);
		if (m_MapMarkerNameLabel)
			m_MapMarkerNameLabel.Show(false);
		if (m_MapMarkerNameBg)
			m_MapMarkerNameBg.Show(false);
		if (m_MapMarkerNameEdit)
			m_MapMarkerNameEdit.Show(false);
		ShowEditUnderline("MapMarkerNameEdit", false);
		if (m_MapAddMarkerBtn)
		{
			m_MapAddMarkerBtn.SetText(SM_PartyLoc.Text(""));
			m_MapAddMarkerBtn.Show(false);
			ShowBtnDressing("MapAddMarkerBtn", false);
		}
		if (m_MapRemoveMarkerBtn)
		{
			m_MapRemoveMarkerBtn.Show(false);
			ShowBtnDressing("MapRemoveMarkerBtn", false);
		}
		if (m_MapColorLabel)
			m_MapColorLabel.Show(false);
		if (m_MapColorBtn)
			m_MapColorBtn.Show(false);
		if (m_MapColorSwatch)
			m_MapColorSwatch.Show(false);
		if (m_MapSelectedText)
			m_MapSelectedText.SetText(SM_PartyLoc.Text("#STR_SMP_00642"));
		if (m_MapShowMeBtn)
		{
			m_MapShowMeBtn.Show(true);
			ShowBtnDressing("MapShowMeBtn", true);
		}
		if (m_MapToggle3DBtn)
		{
			m_MapToggle3DBtn.Show(true);
			ShowBtnDressing("MapToggle3DBtn", true);
		}
		if (m_MapAdminBasesBtn)
		{
			if (m_ShowAdminBaseMarkers)
				SetButtonText(m_MapAdminBasesBtn, "#STR_SMP_00270");
			else
				SetButtonText(m_MapAdminBasesBtn, "#STR_SMP_00271");

			m_MapAdminBasesBtn.Show(showAdminBasesButton);
			ShowBtnDressing("MapAdminBasesBtn", showAdminBasesButton);
		}
		// Геометрия списка задана в layout: TextListboxWidget плохо переносит
		// рантайм-ресайз после инициализации скроллбара.
	}

	protected void RefreshMapMarkerList()
	{
		RefreshMapControls();

		if (!m_MapMarkersList)
			return;

		m_MapMarkersList.ClearItems();
		m_MapMarkerRowIds.Clear();
		m_MapMarkerRowTypes.Clear();

		if (m_MapMarkerMode == MAP_MARKER_MODE_PERSONAL || m_MapMarkerMode == MAP_MARKER_MODE_ALL)
		{
			SM_ClanClientData.EnsurePersonalMapMarkersLoaded();
			foreach (SM_PersonalMapMarker personalMarker : SM_ClanClientData.PersonalMapMarkers)
			{
				if (!personalMarker)
					continue;

				string personalLabel = "#STR_SMP_00609";
				if (personalMarker.Show3D)
					personalLabel = personalLabel + " ·3D";
				int personalRow = m_MapMarkersList.AddItem(SM_PartyLoc.Text(personalMarker.Name), NULL, 0);
				m_MapMarkersList.SetItem(personalRow, SM_PartyLoc.Text(personalLabel), NULL, 1);
				m_MapMarkersList.SetItemColor(personalRow, 0, SM_ClanColors.ResolveColor(personalMarker.Color));
				m_MapMarkerRowIds.Insert(personalMarker.Id);
				m_MapMarkerRowTypes.Insert(MAP_MARKER_ROW_PERSONAL);
			}
		}

		if ((m_MapMarkerMode == MAP_MARKER_MODE_CLAN || m_MapMarkerMode == MAP_MARKER_MODE_ALL) && SM_ClanClientData.HasClan)
		{
			foreach (SM_ClanMapMarker marker : SM_ClanClientData.MapMarkers)
			{
				string clanLabel = "#STR_SMP_00561" + marker.AuthorName;
				if (SM_ClanClientData.IsClanMarker3D(marker.Id))
					clanLabel = clanLabel + " ·3D";
				int row = m_MapMarkersList.AddItem(SM_PartyLoc.Text(marker.Name), NULL, 0);
				m_MapMarkersList.SetItem(row, SM_PartyLoc.Text(clanLabel), NULL, 1);
				m_MapMarkersList.SetItemColor(row, 0, SM_ClanColors.ResolveColor(marker.Color));
				m_MapMarkerRowIds.Insert(marker.Id);
				m_MapMarkerRowTypes.Insert(MAP_MARKER_ROW_CLAN);
			}
		}

		if (m_MapMarkerMode == MAP_MARKER_MODE_SERVER || m_MapMarkerMode == MAP_MARKER_MODE_ALL)
		{
			for (int s = 0; s < SM_ClanClientData.ServerMapMarkers.Count(); s++)
			{
				SM_ServerMapMarker serverMarker = SM_ClanClientData.ServerMapMarkers[s];
				if (!serverMarker)
					continue;

				string serverLabel = "#STR_SMP_00909";
				if (SM_ClanClientData.IsServerMarker3DByIndex(s))
					serverLabel = serverLabel + " ·3D";
				int serverRow = m_MapMarkersList.AddItem(SM_PartyLoc.Text(serverMarker.Name), NULL, 0);
				m_MapMarkersList.SetItem(serverRow, SM_PartyLoc.Text(serverLabel), NULL, 1);
				m_MapMarkersList.SetItemColor(serverRow, 0, SM_ClanColors.ResolveColor(serverMarker.GetColor(SM_ClanColors.GetColor(0))));
				m_MapMarkerRowIds.Insert(s);
				m_MapMarkerRowTypes.Insert(MAP_MARKER_ROW_SERVER);
			}
		}

		if (m_ShowAdminBaseMarkers && SM_ClanClientData.IsAdmin)
		{
			for (int b = 0; b < SM_ClanClientData.AdminBaseMapMarkers.Count(); b++)
			{
				SM_AdminBaseMapMarker adminBaseMarker = SM_ClanClientData.AdminBaseMapMarkers[b];
				if (!adminBaseMarker)
					continue;

				string adminBaseName = adminBaseMarker.ClanName;
				if (adminBaseName == "")
					adminBaseName = "#STR_SMP_00274";

				string adminBaseOwner = adminBaseMarker.OwnerName;
				if (adminBaseOwner == "")
					adminBaseOwner = "#STR_SMP_00678";

				int adminBaseRow = m_MapMarkersList.AddItem(SM_PartyLoc.Text("#STR_SMP_00269" + adminBaseName), NULL, 0);
				m_MapMarkersList.SetItem(adminBaseRow, SM_PartyLoc.Text("#STR_SMP_00324" + adminBaseOwner), NULL, 1);
				m_MapMarkersList.SetItemColor(adminBaseRow, 0, SM_ClanColors.ResolveColor(adminBaseMarker.Color));
				m_MapMarkerRowIds.Insert(b);
				m_MapMarkerRowTypes.Insert(MAP_MARKER_ROW_ADMIN_BASE);
			}
		}

		if (m_MapSelectedText)
		{
			if (m_MapMarkerMode == MAP_MARKER_MODE_SERVER || m_MapMarkerMode == MAP_MARKER_MODE_ALL)
				m_MapSelectedText.SetText(SM_PartyLoc.Text(""));
			else if (m_HasSelectedMapPosition)
				m_MapSelectedText.SetText(SM_PartyLoc.Text("#STR_SMP_01009"));
			else
				m_MapSelectedText.SetText(SM_PartyLoc.Text("#STR_SMP_01010"));
		}

		UpdateMapColorAndToggle();
		RefreshMapControls();
	}

	protected int GetSelectedMarkerRow()
	{
		if (!m_MapMarkersList)
			return -1;
		return m_MapMarkersList.GetSelectedRow();
	}

	// 3D-состояние метки в строке списка: -1 нет строки, 0 выкл, 1 вкл.
	protected int GetRowMarker3DState(int row)
	{
		if (row < 0 || row >= m_MapMarkerRowTypes.Count() || row >= m_MapMarkerRowIds.Count())
			return -1;

		int type = m_MapMarkerRowTypes[row];
		int id = m_MapMarkerRowIds[row];
		bool on = false;
		if (type == MAP_MARKER_ROW_PERSONAL)
			on = SM_ClanClientData.IsPersonalMarker3D(id);
		else if (type == MAP_MARKER_ROW_CLAN)
			on = SM_ClanClientData.IsClanMarker3D(id);
		else if (type == MAP_MARKER_ROW_SERVER)
			on = SM_ClanClientData.IsServerMarker3DByIndex(id);
		else if (type == MAP_MARKER_ROW_ADMIN_BASE)
			return -1;

		if (on)
			return 1;
		return 0;
	}

	// Переключает 3D-показ ВЫБРАННОЙ в списке метки (не всех сразу).
	protected void Toggle3DForSelectedMarker()
	{
		int row = GetSelectedMarkerRow();
		if (row < 0 || row >= m_MapMarkerRowTypes.Count() || row >= m_MapMarkerRowIds.Count())
		{
			if (m_MapSelectedText)
				m_MapSelectedText.SetText(SM_PartyLoc.Text("#STR_SMP_00359"));
			return;
		}

		int type = m_MapMarkerRowTypes[row];
		int id = m_MapMarkerRowIds[row];
		if (type == MAP_MARKER_ROW_PERSONAL)
			SM_ClanClientData.TogglePersonalMarker3D(id);
		else if (type == MAP_MARKER_ROW_CLAN)
			SM_ClanClientData.ToggleClanMarker3D(id);
		else if (type == MAP_MARKER_ROW_SERVER)
			SM_ClanClientData.ToggleServerMarker3DByIndex(id);
		else if (type == MAP_MARKER_ROW_ADMIN_BASE)
		{
			if (m_MapSelectedText)
				m_MapSelectedText.SetText(SM_PartyLoc.Text("#STR_SMP_00412"));
			return;
		}

		RefreshMapMarkerList();
		if (m_MapMarkersList && row < m_MapMarkerRowTypes.Count())
			m_MapMarkersList.SelectRow(row);
		Update3DToggleButton();
	}

	protected void Update3DToggleButton()
	{
		if (!m_MapToggle3DBtn)
			return;

		int state = GetRowMarker3DState(GetSelectedMarkerRow());
		if (state < 0)
			SetButtonText(m_MapToggle3DBtn, "3D");
		else if (state == 1)
			SetButtonText(m_MapToggle3DBtn, "#STR_SMP_00930");
		else
			SetButtonText(m_MapToggle3DBtn, "#STR_SMP_00796");
	}

	protected void UpdateMapColorAndToggle()
	{
		if (m_MapModeLabel)
			m_MapModeLabel.SetText(SM_PartyLoc.Text(GetMapMarkerModeText()));

		if (m_MapColorSwatch)
			m_MapColorSwatch.SetColor(SM_ClanColors.ResolveColor(m_SelectedMarkerColor));

		Update3DToggleButton();
	}

	// Показ/скрытие игрового HUD (vitals, квикбар) — как при открытии меню в ванилле.
	protected void ShowGameHud(bool show)
	{
		Mission mission = GetGame().GetMission();
		if (!mission)
			return;
		if (!mission.GetHud())
			return;
		mission.GetHud().ShowHudUI(show);
		mission.GetHud().ShowQuickbarUI(show);
	}

	// MapWidget нельзя ресайзить в рантайме, поэтому карта всегда держится в полноэкранном holder.
	protected void SetMapHoldersForMode(bool full)
	{
		if (m_MapHolderFull)
			m_MapHolderFull.Show(full);
		m_ClanMap = m_ClanMapFull;
		m_ActiveMapHolder = m_MapHolderFull;
	}

	// В фуллскрене заголовок/подсказка висели бы посреди карты — прячем их.
	protected void ShowMapHeaderTexts(bool show)
	{
		if (m_MapHeaderText)
			m_MapHeaderText.Show(show);
		if (m_MapHintText)
			m_MapHintText.Show(show);
	}

	// Запоминает ИСХОДНУЮ логическую позицию панели (через GetPos — тот же базис,
	// что и SetPos, поэтому возврат точный). Первый вызов всегда из встроенного
	// состояния, так что ловит штатную позицию.
	protected void CaptureSidebarOrigin()
	{
		if (m_SidebarOrigCaptured || !m_MapMarkersBg)
			return;
		m_MapMarkersBg.GetPos(m_SidebarOrigX, m_SidebarOrigY);
		m_SidebarOrigCaptured = true;
	}

	protected void DockSidebarRight()
	{
		if (!m_MapMarkersBg || !m_PanelMap)
			return;
		CaptureSidebarOrigin();
		if (layoutRoot)
			layoutRoot.Update();

		float sw;
		float sh;
		m_PanelMap.GetScreenSize(sw, sh);
		float pw;
		float ph;
		m_MapMarkersBg.GetScreenSize(pw, ph);
		float px;
		float py;
		m_MapMarkersBg.GetScreenPos(px, py);
		float lw;
		float lh;
		m_MapMarkersBg.GetSize(lw, lh);
		if (sw <= 1 || pw <= 1 || lw <= 0.001)
			return;

		float scale = pw / lw;
		float margin = sw * 0.012;
		float deltaScreen = (sw - margin) - (px + pw);
		if (deltaScreen <= 1)
			return;

		float curX;
		float curY;
		m_MapMarkersBg.GetPos(curX, curY);
		m_MapMarkersBg.SetPos(curX + deltaScreen / scale, curY);
	}

	// Возвращает панель меток на штатное место — ровно туда, где она
	// была при загрузке layout.
	protected void RestoreSidebarPosition()
	{
		CaptureSidebarOrigin();
		if (m_MapMarkersBg && m_SidebarOrigCaptured)
			m_MapMarkersBg.SetPos(m_SidebarOrigX, m_SidebarOrigY);
	}

	protected void SetMapFullscreen(bool full)
	{
		m_MapFullscreen = full;
		SetMapHoldersForMode(full);
		ShowGameHud(!full);
		ShowMapHeaderTexts(!full);
		if (!full)
			RestoreSidebarPosition();

		if (m_MapCloseBtn)
			m_MapCloseBtn.Show(full);

		RefreshMapView();
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.RefreshMapView, 50, false);
	}

	// Безопасный выход из полноэкранного режима (при смене вкладки / закрытии меню).
	protected void ExitMapFullscreen()
	{
		if (!m_MapFullscreen)
			return;

		m_MapFullscreen = false;
		SetMapHoldersForMode(false);
		ShowGameHud(true);
		ShowMapHeaderTexts(true);
		RestoreSidebarPosition();

		if (m_MapCloseBtn)
			m_MapCloseBtn.Show(false);
	}


	void RefreshAll()
	{
		if (m_InviteMode && (!SM_ClanClientData.HasClan || SM_ClanClientData.MyRank < SM_ClanClientData.GetActionRank(SM_ClanAction.INVITE)))
			m_InviteMode = false;
		if (m_DescMode && !SM_ClanClientData.HasClan)
			m_DescMode = false;
		if (m_PermsMode && !CanOpenPermsPanel())
			m_PermsMode = false;
		if (m_CurrentTab >= 4 && m_CurrentTab != 6 && m_CurrentTab != 7 && m_CurrentTab != 8 && m_CurrentTab != 10 && m_CurrentTab != 11 && m_CurrentTab != 13 && m_CurrentTab != 14 && !SM_ClanClientData.HasClan)
			m_CurrentTab = 0;
		if (m_CurrentTab == 2 && SM_ClanClientData.HasClan && SM_ClanClientData.MyRank < SM_ClanClientData.GetActionRank(SM_ClanAction.APPLICATIONS))
			m_CurrentTab = 0;
		if (m_CurrentTab == 3 && !SM_ClanClientData.TopsEnabled)
			m_CurrentTab = 0;
		if (m_CurrentTab == 7 && !SM_ClanClientData.MarketEnabled)
			m_CurrentTab = 0;
		if (m_CurrentTab == 8 && !SM_ClanClientData.ServerMarketEnabled)
			m_CurrentTab = 0;
		if (m_CurrentTab == 4 && !SM_ClanClientData.TreasuryEnabled)
			m_CurrentTab = 0;
		if (m_CurrentTab == 9 && !SM_ClanClientData.StorageEnabled)
			m_CurrentTab = 0;
		if (m_CurrentTab == 10 && !SM_ClanClientData.AuctionEnabled)
			m_CurrentTab = 0;
		if (m_CurrentTab == 11 && !SM_ClanClientData.AchievementsEnabled)
			m_CurrentTab = 0;
		if (m_CurrentTab == 13 && !SM_ClanClientData.PlayerExperienceEnabled)
			m_CurrentTab = 0;
		if (m_CurrentTab == 14 && !SM_ClanClientData.ContractsEnabled)
			m_CurrentTab = 0;
		if (m_CurrentTab == 15 && !SM_ClanClientData.IsAdmin)
			m_CurrentTab = 0;
		if (m_CurrentTab == 6 && !SM_ClanClientData.MapEnabled)
			m_CurrentTab = 0;

		ApplyCreateLogo();
		RefreshMoney();
		RefreshOnlineWidget();
		RefreshTabs();
		ShowTab();
		RefreshMyClan();
		RefreshInvites();
		RefreshClans();
		RefreshPlayers();
		RefreshTops();
		RefreshTreasury();
		RefreshLog();
		RefreshAdminChatLog();
		RefreshDescPanel();
		RefreshPerms();
		RefreshMarket();
		RefreshServerMarket();
		RefreshAuction();
		RefreshAchievements();
		RefreshPlayerTitles();
		RefreshContracts();
		RefreshStorage();
		RefreshMapMarkerList();

		if (m_CurrentTab == 6)
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.RefreshMapView, 50, false);
	}

	protected void RefreshTabs()
	{
		bool canManageApps = SM_ClanClientData.HasClan && SM_ClanClientData.MyRank >= SM_ClanClientData.GetActionRank(SM_ClanAction.APPLICATIONS);

		if (SM_ClanClientData.HasClan)
		{
			m_TabInvitesBtn.Show(canManageApps);
			int appCount = SM_ClanClientData.Applications.Count();
			if (appCount > 0)
				SetTabText(2, m_TabInvitesBtn, "#STR_SMP_00454" + appCount + ")");
			else
				SetTabText(2, m_TabInvitesBtn, "#STR_SMP_00453");
		}
		else
		{
			m_TabInvitesBtn.Show(true);
			int inviteCount = SM_ClanClientData.Invites.Count();
			if (inviteCount > 0)
				SetTabText(2, m_TabInvitesBtn, "#STR_SMP_00834" + inviteCount + ")");
			else
				SetTabText(2, m_TabInvitesBtn, "#STR_SMP_00833");
		}

		m_TabTopsBtn.Show(SM_ClanClientData.TopsEnabled);
		m_TabTreasuryBtn.Show(SM_ClanClientData.HasClan && SM_ClanClientData.TreasuryEnabled);
		m_TabLogBtn.Show(SM_ClanClientData.HasClan);
		if (m_TabSettingsBtn)
			m_TabSettingsBtn.Show(true);
		if (m_TabMapBtn)
			m_TabMapBtn.Show(false);
		m_TabMarketBtn.Show(SM_ClanClientData.MarketEnabled);
		m_TabServerMarketBtn.Show(SM_ClanClientData.ServerMarketEnabled);
		m_TabStorageBtn.Show(SM_ClanClientData.HasClan && SM_ClanClientData.StorageEnabled);
		if (m_TabAuctionBtn)
			m_TabAuctionBtn.Show(SM_ClanClientData.AuctionEnabled);
		if (m_TabAchievementsBtn)
			m_TabAchievementsBtn.Show(SM_ClanClientData.AchievementsEnabled);
		if (m_TabTitlesBtn)
			m_TabTitlesBtn.Show(SM_ClanClientData.PlayerExperienceEnabled);
		if (m_TabContractsBtn)
			m_TabContractsBtn.Show(SM_ClanClientData.ContractsEnabled);
		if (m_TabAdminChatBtn)
			m_TabAdminChatBtn.Show(SM_ClanClientData.IsAdmin);
	}

	protected void ShowTab()
	{
		SetFocus(NULL);

		if (m_CurrentTab != 6)
			ExitMapFullscreen();

		m_PanelNoClan.Show(false);
		m_PanelMyClan.Show(false);
		m_PanelInvitePlayers.Show(false);
		m_PanelClans.Show(false);
		m_PanelInvites.Show(false);
		m_PanelTops.Show(false);
		m_PanelTreasury.Show(false);
		m_PanelLog.Show(false);
		if (m_PanelSettings)
			m_PanelSettings.Show(false);
		HideMapMarkerEditor();
		m_PanelMap.Show(false);
		if (m_MapMarkerWidgetLayer)
			m_MapMarkerWidgetLayer.Show(false);
		if (m_MapCoordsText)
			m_MapCoordsText.Show(false);
		m_PanelMarket.Show(false);
		m_PanelServerMarket.Show(false);
		if (m_PanelAuction)
			m_PanelAuction.Show(false);
		if (m_PanelAchievements)
			m_PanelAchievements.Show(false);
		if (m_PanelTitles)
			m_PanelTitles.Show(false);
		if (m_PanelContracts)
			m_PanelContracts.Show(false);
		if (m_PanelAdminChat)
			m_PanelAdminChat.Show(false);
		HideAdminChatMutePanel();
		m_PanelStorage.Show(false);
		m_PanelDescEdit.Show(false);
		m_PanelPerms.Show(false);
		m_PanelClanInfo.Show(false);

		for (int i = 0; i < m_TabUnderlines.Count(); i++)
		{
			if (m_TabUnderlines[i])
				m_TabUnderlines[i].Show(i == m_CurrentTab);
		}
		UpdateTabVisuals();

		if (m_CurrentTab == 0)
		{
			if (m_InviteMode)
				m_PanelInvitePlayers.Show(true);
			else if (m_DescMode)
				m_PanelDescEdit.Show(true);
			else if (m_PermsMode)
				m_PanelPerms.Show(true);
			else if (SM_ClanClientData.HasClan)
				m_PanelMyClan.Show(true);
			else
				m_PanelNoClan.Show(true);
		}
		else if (m_CurrentTab == 1)
		{
			if (m_InfoMode)
				m_PanelClanInfo.Show(true);
			else
				m_PanelClans.Show(true);
		}
		else if (m_CurrentTab == 2)
		{
			m_PanelInvites.Show(true);
		}
		else if (m_CurrentTab == 3)
		{
			m_PanelTops.Show(true);
		}
		else if (m_CurrentTab == 4)
		{
			m_PanelTreasury.Show(true);
		}
		else if (m_CurrentTab == 5)
		{
			m_PanelLog.Show(true);
		}
		else if (m_CurrentTab == 6 && SM_ClanClientData.MapEnabled)
		{
			m_PanelMap.Show(true);
			SetMapFullscreen(true);
			FocusMenuInput();
		}
		else if (m_CurrentTab == 8)
		{
			m_PanelServerMarket.Show(true);
		}
		else if (m_CurrentTab == 9)
		{
			m_PanelStorage.Show(true);
		}
		else if (m_CurrentTab == 10)
		{
			m_PanelAuction.Show(true);
		}
		else if (m_CurrentTab == 11)
		{
			m_PanelAchievements.Show(true);
		}
		else if (m_CurrentTab == 12)
		{
			if (m_PanelSettings)
				m_PanelSettings.Show(true);
			CloseMembersOverlay();
			RefreshSettingsPanel();
		}
		else if (m_CurrentTab == 13)
		{
			if (m_PanelTitles)
				m_PanelTitles.Show(true);
			RefreshPlayerTitles();
		}
		else if (m_CurrentTab == 14)
		{
			if (m_PanelContracts)
				m_PanelContracts.Show(true);
			RefreshContracts();
			RequestContracts();
		}
		else if (m_CurrentTab == 15)
		{
			if (m_PanelAdminChat)
				m_PanelAdminChat.Show(true);
			RefreshAdminChatLog();
			RequestAdminChatLog();
		}
		else
		{
			m_PanelMarket.Show(true);
		}

		if (layoutRoot)
			layoutRoot.Update();
	}

	protected string GetSettingName(int row)
	{
		if (row == 0) return "#STR_SMP_00979";
		if (row == 1) return "#STR_SMP_00982";
		if (row == 2) return "#STR_SMP_00481";
		if (row == 3) return "#STR_SMP_00446";
		if (row == 4) return "#STR_SMP_00866";
		if (row == 5) return "#STR_SMP_00941";
		if (row == 6) return "#STR_SMP_00942";
		if (row == 7) return "#STR_SMP_00227";
		if (row == 8) return "#STR_SMP_00573";
		if (row == 9) return "#STR_SMP_00474";
		if (row == 10) return "#STR_SMP_00865";
		if (row == 11) return "#STR_SMP_00939";
		if (row == 12) return "#STR_SMP_00940";
		if (row == 13) return "#STR_SMP_00408";
		if (row == 14) return "#STR_SMP_00409";
		if (row == 15) return "#STR_SMP_00631";
		if (row == 16) return "#STR_SMP_00225";
		if (row == 17) return "#STR_SMP_00410";
		if (row == 18) return "#STR_SMP_00672";
		if (row == 19) return "#STR_SMP_00799";
		if (row == 20) return "#STR_SMP_00794";
		return "";
	}

	protected bool IsToggleSetting(int row)
	{
		return (row == 1 || row == 2 || row == 7 || row == 8 || row == 15 || row == 16 || row == 17 || row == 18 || row == 19);
	}

	protected bool GetToggleState(int row)
	{
		SM_ClientSettingsData s = SM_ClanClientData.ClientSettings;
		if (!s)
			return false;
		if (row == 1) return s.ChatTimestamps;
		if (row == 2) return s.ChatHistoryEnabled;
		if (row == 7) return s.HudEnabled;
		if (row == 8) return s.HudCompact;
		if (row == 15) return s.MarkersEnabled;
		if (row == 16) return s.Markers3D;
		if (row == 17) return s.HudShowDistance;
		if (row == 18) return s.HudShowDirection;
		if (row == 19) return s.HudShowSelf;
		return false;
	}

	protected void ToggleSetting(int row)
	{
		SM_ClanClientData.EnsureClientSettingsLoaded();
		SM_ClientSettingsData s = SM_ClanClientData.ClientSettings;
		if (!s)
			return;
		bool nv = !GetToggleState(row);
		if (row == 1)
			s.ChatTimestamps = nv;
		else if (row == 2)
			s.ChatHistoryEnabled = nv;
		else if (row == 7)
			s.HudEnabled = nv;
		else if (row == 8)
			s.HudCompact = nv;
		else if (row == 15)
			s.MarkersEnabled = nv;
		else if (row == 16)
			s.Markers3D = nv;
		else if (row == 17)
			s.HudShowDistance = nv;
		else if (row == 18)
			s.HudShowDirection = nv;
		else if (row == 19)
			s.HudShowSelf = nv;
		SM_ClanClientData.ApplyClientSettings();
		SM_ClanClientData.SaveClientSettings();
		RefreshSettingsPanel();
	}

	protected string GetSettingValue(int row)
	{
		SM_ClientSettingsData s = SM_ClanClientData.ClientSettings;
		if (!s)
			return "";
		if (row == 0) return s.ChatVisibleMessages.ToString();
		if (row == 3) return s.ChatFadeSeconds.ToString() + "#STR_SMP_00081";
		if (row == 4) return s.ChatOpacity.ToString() + "%";
		if (row == 5) return s.ChatOffsetX.ToString();
		if (row == 6) return s.ChatOffsetY.ToString();
		if (row == 9) return s.HudMaxVisibleMembers.ToString();
		if (row == 10) return s.HudOpacity.ToString() + "%";
		if (row == 11) return s.HudOffsetX.ToString();
		if (row == 12) return s.HudOffsetY.ToString();
		if (row == 13) return GetDistanceSettingValue(s.MarkerDistance);
		if (row == 14) return GetDistanceSettingValue(s.PingMarkerDistance);
		return "";
	}

	protected string GetDistanceSettingValue(int distance)
	{
		if (distance <= 0)
			return "#STR_SMP_00272";
		return distance.ToString() + "#STR_SMP_00057";
	}

	protected void BuildSettingsRows()
	{
		if (m_SettingsBuilt)
			return;
		if (!m_SettingsRowsRoot)
			return;
		m_SettingsBuilt = true;

		m_SettingDecBtns.Clear();
		m_SettingIncBtns.Clear();
		m_SettingValueTexts.Clear();
		m_SettingToggles.Clear();
		m_SettingToggleIcons.Clear();

		int half = (SETTINGS_COUNT + 1) / 2;
		for (int i = 0; i < SETTINGS_COUNT; i++)
		{
			Widget row = GetGame().GetWorkspace().CreateWidgets("SM_PartyMod/GUI/layouts/SM_SettingRow.layout", m_SettingsRowsRoot);
			if (!row)
			{
				m_SettingValueTexts.Insert(null);
				m_SettingDecBtns.Insert(null);
				m_SettingIncBtns.Insert(null);
				m_SettingToggles.Insert(null);
				m_SettingToggleIcons.Insert(null);
				continue;
			}

			int col = 0;
			int yIdx = i;
			if (i >= half)
			{
				col = 1;
				yIdx = i - half;
			}
			// Горизонталь — относительные доли (пиксельный SetPos по X «плывёт»
			// из-за масштаба экрана); правый столбец прижимаем к правому краю.
			// Вертикаль оставляем в пикселях.
			float colWFrac = 0.49558; // 448/904 — ширина столбца как доля корня
			float xFrac = 0.0;
			if (col == 1)
				xFrac = 1.0 - colWFrac;
			row.ClearFlags(WidgetFlags.HEXACTPOS);
			row.ClearFlags(WidgetFlags.HEXACTSIZE);
			row.SetSize(colWFrac, 30);
			row.SetPos(xFrac, yIdx * 32);

			TextWidget label = TextWidget.Cast(row.FindAnyWidget("SettingRowLabel"));
			if (label)
				label.SetText(SM_PartyLoc.Text(GetSettingName(i)));

			TextWidget val = TextWidget.Cast(row.FindAnyWidget("SettingRowValue"));
			ButtonWidget dec = ButtonWidget.Cast(row.FindAnyWidget("SettingRowDec"));
			ButtonWidget inc = ButtonWidget.Cast(row.FindAnyWidget("SettingRowInc"));
			Widget decBg = row.FindAnyWidget("SettingRowDecBg");
			Widget incBg = row.FindAnyWidget("SettingRowIncBg");
			ButtonWidget toggle = ButtonWidget.Cast(row.FindAnyWidget("SettingRowToggle"));
			ImageWidget toggleIcon = ImageWidget.Cast(row.FindAnyWidget("SettingRowToggleIcon"));
			if (dec)
				dec.SetTextColor(ARGB(255, 250, 240, 228));
			if (inc)
				inc.SetTextColor(ARGB(255, 250, 240, 228));
			// Убираем белую обводку/заливку фокуса: style EmptyHighlight рисует
			// WhitePixel в состоянии Focus, и вокруг иконки тумблера это выглядит
			// как белая рамка. Меню мышиное — фокус кнопкам не нужен.
			if (dec) dec.SetFlags(WidgetFlags.NOFOCUS);
			if (inc) inc.SetFlags(WidgetFlags.NOFOCUS);
			if (toggle)
			{
				toggle.SetFlags(WidgetFlags.NOFOCUS);
				ClearSettingToggleFrame(toggle);
			}
			if (toggleIcon)
				toggleIcon.SetColor(ARGB(255, 255, 255, 242));

			if (IsToggleSetting(i))
			{
				if (val) val.Show(false);
				if (dec) dec.Show(false);
				if (inc) inc.Show(false);
				if (decBg) decBg.Show(false);
				if (incBg) incBg.Show(false);
				if (toggle) toggle.Show(true);
				if (toggleIcon) toggleIcon.Show(true);
			}
			else if (i == SETTINGS_COUNT - 1)
			{
				// «Показ участников» — одна кнопка «Показать» на всю правую часть
				// строки (без отдельного текста-значения и стрелки «>»).
				if (toggle) toggle.Show(false);
				if (toggleIcon) toggleIcon.Show(false);
				if (val) val.Show(false);
				if (dec) dec.Show(false);
				if (decBg) decBg.Show(false);
				if (incBg)
				{
					incBg.Show(true);
					incBg.SetPos(0.54018, 3);
					incBg.SetSize(0.39286, 24);
				}
				if (inc)
				{
					inc.Show(true);
					inc.SetPos(0.54018, 3);
					inc.SetSize(0.39286, 24);
					inc.SetText(SM_PartyLoc.Text("#STR_SMP_00795"));
				}
			}
			else
			{
				if (toggle) toggle.Show(false);
				if (toggleIcon) toggleIcon.Show(false);
			}

			m_SettingValueTexts.Insert(val);
			m_SettingDecBtns.Insert(dec);
			m_SettingIncBtns.Insert(inc);
			m_SettingToggles.Insert(toggle);
			m_SettingToggleIcons.Insert(toggleIcon);
		}
	}

	protected void BuildMemberOverlayRows()
	{
		if (m_MembersBuilt)
			return;
		if (!m_MembersRowsRoot)
			return;
		m_MembersBuilt = true;

		m_MemberRowWidgets.Clear();
		m_MemberRowDec.Clear();
		m_MemberRowInc.Clear();
		m_MemberRowName.Clear();
		m_MemberRowValue.Clear();
		m_HudSelUids.Clear();
		for (int j = 0; j < MAX_MEMBER_ROWS; j++)
		{
			Widget mrow = GetGame().GetWorkspace().CreateWidgets("SM_PartyMod/GUI/layouts/SM_SettingRow.layout", m_MembersRowsRoot);
			if (!mrow)
				continue;
			mrow.SetPos(0, j * 32);
			mrow.Show(false);
			ButtonWidget mdec = ButtonWidget.Cast(mrow.FindAnyWidget("SettingRowDec"));
			ButtonWidget minc = ButtonWidget.Cast(mrow.FindAnyWidget("SettingRowInc"));
			if (mdec)
			{
				mdec.SetTextColor(ARGB(255, 250, 240, 228));
				mdec.SetFlags(WidgetFlags.NOFOCUS);
			}
			if (minc)
			{
				minc.SetTextColor(ARGB(255, 250, 240, 228));
				minc.SetFlags(WidgetFlags.NOFOCUS);
			}
			m_MemberRowWidgets.Insert(mrow);
			m_MemberRowName.Insert(TextWidget.Cast(mrow.FindAnyWidget("SettingRowLabel")));
			m_MemberRowValue.Insert(TextWidget.Cast(mrow.FindAnyWidget("SettingRowValue")));
			m_MemberRowDec.Insert(mdec);
			m_MemberRowInc.Insert(minc);
			m_HudSelUids.Insert("");
		}
	}

	protected void RefreshSettingsPanel()
	{
		SM_ClanClientData.EnsureClientSettingsLoaded();
		BuildSettingsRows();
		for (int i = 0; i < SETTINGS_COUNT; i++)
		{
			if (IsToggleSetting(i))
			{
				if (i < m_SettingToggleIcons.Count() && m_SettingToggleIcons[i])
				{
					string ipath = "SM_PartyMod\\GUI\\icons\\toggle_off.paa";
					if (GetToggleState(i))
						ipath = "SM_PartyMod\\GUI\\icons\\toggle_on.paa";
					m_SettingToggleIcons[i].LoadImageFile(0, ipath);
					m_SettingToggleIcons[i].SetImage(0);
				}
			}
			else if (i < m_SettingValueTexts.Count() && m_SettingValueTexts[i])
				m_SettingValueTexts[i].SetText(SM_PartyLoc.Text(GetSettingValue(i)));
		}
	}

	protected void RefreshMemberOverlay()
	{
		BuildMemberOverlayRows();
		int rowIdx = 0;
		int total = SM_ClanClientData.HudMembers.Count();
		for (int k = 0; k < total; k++)
		{
			SM_ClanMemberView mv = SM_ClanClientData.HudMembers[k];
			if (!mv)
				continue;
			// Себя в списке не показываем — для этого есть отдельная настройка
			// «Показывать себя», иначе игрок дублируется тут и там.
			if (mv.Uid == SM_ClanClientData.MyUid)
				continue;
			if (rowIdx >= m_MemberRowWidgets.Count() || rowIdx >= 11)
				break;

			if (m_MemberRowWidgets[rowIdx])
				m_MemberRowWidgets[rowIdx].Show(true);
			m_HudSelUids[rowIdx] = mv.Uid;
			if (m_MemberRowName[rowIdx])
				m_MemberRowName[rowIdx].SetText(SM_PartyLoc.Text(mv.Name));
			string vtext = "#STR_SMP_00798";
			if (SM_ClanClientData.IsHudMemberHidden(mv.Uid))
				vtext = "#STR_SMP_00928";
			if (m_MemberRowValue[rowIdx])
				m_MemberRowValue[rowIdx].SetText(SM_PartyLoc.Text(vtext));
			rowIdx++;
		}

		// Прячем оставшиеся строки пула.
		for (int j = rowIdx; j < m_MemberRowWidgets.Count(); j++)
		{
			if (m_MemberRowWidgets[j])
				m_MemberRowWidgets[j].Show(false);
			if (j < m_HudSelUids.Count())
				m_HudSelUids[j] = "";
		}
	}

	protected void OpenMembersOverlay()
	{
		SM_ClanClientData.EnsureClientSettingsLoaded();
		BuildMemberOverlayRows();
		if (m_PanelMembers)
			m_PanelMembers.Show(true);
		RefreshMemberOverlay();
	}

	protected void CloseMembersOverlay()
	{
		if (m_PanelMembers)
			m_PanelMembers.Show(false);
	}

	protected int SettingsClamp(int v, int lo, int hi)
	{
		if (v < lo)
			return lo;
		if (v > hi)
			return hi;
		return v;
	}

	protected int GetHudSettingsMaxX()
	{
		int screenW;
		int screenH;
		GetScreenSize(screenW, screenH);

		int safeWidth = HUD_PANEL_SAFE_WIDTH;
		if (SM_ClanClientData.ClientSettings && SM_ClanClientData.ClientSettings.HudCompact)
			safeWidth = HUD_PANEL_SAFE_COMPACT_WIDTH;

		int maxX = screenW - safeWidth;
		if (maxX < 0)
			maxX = 0;

		return maxX;
	}

	protected int GetHudSettingsMaxY()
	{
		int screenW;
		int screenH;
		GetScreenSize(screenW, screenH);

		int safeHeight = HUD_PANEL_SAFE_ROW_HEIGHT;
		if (SM_ClanClientData.ClientSettings && SM_ClanClientData.ClientSettings.HudCompact)
			safeHeight = HUD_PANEL_SAFE_COMPACT_ROW_HEIGHT;

		int maxY = screenH - safeHeight;
		if (maxY < 0)
			maxY = 0;

		return maxY;
	}

	protected void AdjustSetting(int row, int dir)
	{
		if (row == SETTINGS_COUNT - 1)
		{
			OpenMembersOverlay();
			return;
		}

		SM_ClanClientData.EnsureClientSettingsLoaded();
		SM_ClientSettingsData s = SM_ClanClientData.ClientSettings;
		if (!s)
			return;

		if (row == 0)
			s.ChatVisibleMessages = SettingsClamp(s.ChatVisibleMessages + dir, 1, 18);
		else if (row == 3)
			s.ChatFadeSeconds = SettingsClamp(s.ChatFadeSeconds + dir, 2, 60);
		else if (row == 4)
			s.ChatOpacity = SettingsClamp(s.ChatOpacity + dir * 10, 20, 100);
		else if (row == 5)
			s.ChatOffsetX = SettingsClamp(s.ChatOffsetX + dir * 10, -700, 700);
		else if (row == 6)
			s.ChatOffsetY = SettingsClamp(s.ChatOffsetY + dir * 10, -400, 400);
		else if (row == 9)
			s.HudMaxVisibleMembers = SettingsClamp(s.HudMaxVisibleMembers + dir, 1, 20);
		else if (row == 10)
			s.HudOpacity = SettingsClamp(s.HudOpacity + dir * 10, 20, 100);
		else if (row == 11)
			s.HudOffsetX = SettingsClamp(s.HudOffsetX + dir * 10, 0, GetHudSettingsMaxX());
		else if (row == 12)
			s.HudOffsetY = SettingsClamp(s.HudOffsetY + dir * 10, 0, GetHudSettingsMaxY());
		else if (row == 13)
			s.MarkerDistance = SettingsClamp(s.MarkerDistance + dir * 50, 0, 2000);
		else if (row == 14)
			s.PingMarkerDistance = SettingsClamp(s.PingMarkerDistance + dir * 50, 0, 2000);

		SM_ClanClientData.ApplyClientSettings();
		SM_ClanClientData.SaveClientSettings();
		RefreshSettingsPanel();
	}

	// Возвращает участников в порядке: сначала онлайн, потом оффлайн (порядок внутри групп сохраняется).
	protected array<SM_ClanMemberView> OrderMembersOnlineFirst(array<ref SM_ClanMemberView> src)
	{
		array<SM_ClanMemberView> ordered = new array<SM_ClanMemberView>;
		if (!src)
			return ordered;
		foreach (SM_ClanMemberView online : src)
		{
			if (online && online.Online)
				ordered.Insert(online);
		}
		foreach (SM_ClanMemberView offline : src)
		{
			if (offline && !offline.Online)
				ordered.Insert(offline);
		}
		return ordered;
	}

	protected void RefreshMyClan()
	{
		if (!SM_ClanClientData.HasClan)
		{
			m_CreateHintText.SetColor(ARGB(255, 255, 255, 242));
			m_CreateHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00664"));
			UpdateCreateCounters();
			ApplyCreateLogo();
			return;
		}

		int clanColor = SM_ClanClientData.GetClanColor();

		string title = SM_ClanClientData.ClanName;
		if (SM_ClanClientData.ClanTag != "")
			title = "[" + SM_ClanClientData.ClanTag + "] " + title;
		m_ClanTitleText.SetText(SM_PartyLoc.Text(title));
		m_ClanTitleText.SetColor(clanColor);
		if (SM_ClanClientData.IsAdminMode)
			m_MyRankText.SetText(SM_PartyLoc.Text("#STR_SMP_00310"));
		else
			m_MyRankText.SetText(SM_PartyLoc.Text("#STR_SMP_00307" + SM_ClanClientData.GetRankName(SM_ClanClientData.MyRank)));

		if (m_ClanLevelText)
		{
			string levelInfo = "#STR_SMP_01042" + SM_ClanClientData.ClanLevel.ToString();
			if (!SM_ClanClientData.ClanLevelsEnabled)
				levelInfo = levelInfo + "#STR_SMP_00021";
			if (SM_ClanClientData.StorageEnabled)
				levelInfo = levelInfo + "#STR_SMP_00005" + SM_ClanClientData.StorageUsed.ToString() + "/" + SM_ClanClientData.StorageCap.ToString();
			if (SM_ClanClientData.MarketEnabled)
				levelInfo = levelInfo + "#STR_SMP_00004" + SM_ClanClientData.LotCap.ToString();
			m_ClanLevelText.SetText(SM_PartyLoc.Text(levelInfo));
		}

		m_MembersList.ClearItems();
		m_MemberRowUids.Clear();
		if (m_HdrMemberTitle)
		{
			if (SM_ClanClientData.PlayerExperienceClanListEnabled)
				m_HdrMemberTitle.SetText(SM_PartyLoc.Text("#STR_SMP_00457"));
			else
				m_HdrMemberTitle.SetText(SM_PartyLoc.Text(""));
		}

		int leaderRank = SM_ClanClientData.GetLeaderRank();

		array<SM_ClanMemberView> orderedMembers = OrderMembersOnlineFirst(SM_ClanClientData.Members);
		foreach (SM_ClanMemberView member : orderedMembers)
		{
			string name = member.Name;
			if (member.Uid == SM_ClanClientData.MyUid)
				name = name + "#STR_SMP_00016";

			string lastSeen = member.LastSeen;
			if (lastSeen == "")
				lastSeen = "-";
			string status = lastSeen;
			if (member.Online)
			{
				status = "#STR_SMP_00726";
			}
			string playerTitle = "";
			if (SM_ClanClientData.PlayerExperienceClanListEnabled)
				playerTitle = member.PlayerTitle;
			if (playerTitle == "")
				playerTitle = "-";

			int row = m_MembersList.AddItem(SM_PartyLoc.Text(name), NULL, 0);
			m_MembersList.SetItem(row, SM_PartyLoc.Text(SM_ClanClientData.GetRankName(member.Rank)), NULL, 1);
			m_MembersList.SetItem(row, SM_PartyLoc.Text(playerTitle), NULL, 2);
			m_MembersList.SetItem(row, SM_PartyLoc.Text(lastSeen), NULL, 3);
			m_MembersList.SetItem(row, SM_PartyLoc.Text(status), NULL, 4);

			if (!member.Online)
			{
				m_MembersList.SetItemColor(row, 0, ARGB(255, 143, 131, 114));
				m_MembersList.SetItemColor(row, 1, ARGB(255, 143, 131, 114));
				m_MembersList.SetItemColor(row, 2, ARGB(255, 143, 131, 114));
				m_MembersList.SetItemColor(row, 3, ARGB(255, 143, 131, 114));
				m_MembersList.SetItemColor(row, 4, ARGB(255, 143, 131, 114));
			}
			else if (member.Rank == leaderRank)
			{
				m_MembersList.SetItemColor(row, 1, ARGB(255, 232, 181, 69));
			}
			if (member.Online && SM_ClanClientData.PlayerExperienceClanListEnabled && member.PlayerTitleColor != 0)
				m_MembersList.SetItemColor(row, 2, member.PlayerTitleColor);

			m_MemberRowUids.Insert(member.Uid);
		}

		bool isLeader = (SM_ClanClientData.MyRank == leaderRank || SM_ClanClientData.IsAdminMode);
		bool canColor = SM_ClanClientData.MyRank >= SM_ClanClientData.GetActionRank(SM_ClanAction.COLOR) || SM_ClanClientData.IsAdminMode;
		m_InviteBtn.Show(SM_ClanClientData.MyRank >= SM_ClanClientData.GetActionRank(SM_ClanAction.INVITE));
		m_KickBtn.Show(SM_ClanClientData.MyRank >= SM_ClanClientData.GetActionRank(SM_ClanAction.KICK));
		m_PromoteBtn.Show(isLeader);
		m_DemoteBtn.Show(isLeader);
		m_LeaveBtn.Show(!SM_ClanClientData.IsAdminMode);
		m_DisbandBtn.Show(isLeader && !SM_ClanClientData.IsAdminMode);
		m_DescBtn.Show(true);
		m_PermsBtn.Show(CanOpenPermsPanel());
		m_AdminLeaveClanBtn.Show(SM_ClanClientData.IsAdminMode);

		m_ColorLabel.Show(canColor);
		int clanColorIndex = SM_ClanClientData.ClanColorIndex;
		if (clanColorIndex < 0 || clanColorIndex >= SM_ClanColors.Count())
			clanColorIndex = 0;
		for (int c = 0; c < m_ColorBtns.Count(); c++)
		{
			if (m_ColorBtns[c])
				m_ColorBtns[c].Show(canColor);
			if (m_ColorSwatches[c])
			{
				if (c == clanColorIndex)
					m_ColorSwatches[c].SetAlpha(1.0);
				else
					m_ColorSwatches[c].SetAlpha(0.45);
			}
			if (m_ColorSelected[c])
			{
				m_ColorSelected[c].Show(canColor && c == clanColorIndex);
				if (c == clanColorIndex)
					m_ColorSelected[c].SetAlpha(1.0);
				else
					m_ColorSelected[c].SetAlpha(0.0);
			}
		}
	}

	void RefreshClans()
	{
		m_ClansList.ClearItems();
		m_ClanRowNames.Clear();
		foreach (SM_ClanListEntry entry : SM_ClanClientData.ClanList)
		{
			int row = m_ClansList.AddItem(SM_PartyLoc.Text(entry.Name), NULL, 0);
			m_ClansList.SetItem(row, SM_PartyLoc.Text(entry.Tag), NULL, 1);
			m_ClansList.SetItem(row, SM_PartyLoc.Text(entry.MemberCount.ToString()), NULL, 2);
			m_ClansList.SetItem(row, SM_PartyLoc.Text(entry.LeaderName), NULL, 3);
			string leaderTitle = entry.LeaderTitle;
			if (leaderTitle == "")
				leaderTitle = "-";
			m_ClansList.SetItem(row, SM_PartyLoc.Text(leaderTitle), NULL, 4);
			if (entry.LeaderTitleColor != 0)
				m_ClansList.SetItemColor(row, 4, entry.LeaderTitleColor);
			m_ClanRowNames.Insert(entry.Name);
		}
	}

	protected void RefreshInvites()
	{
		m_InvitesList.ClearItems();
		m_InviteRowClans.Clear();
		m_ApplicationRowUids.Clear();

		if (SM_ClanClientData.HasClan)
		{
			m_HdrInvClan.SetText(SM_PartyLoc.Text("#STR_SMP_00463"));
			m_HdrInvFrom.SetText(SM_PartyLoc.Text("#STR_SMP_00399"));
			SetButtonText(m_AcceptInviteBtn, "#STR_SMP_00845");
			SetButtonText(m_DeclineInviteBtn, "#STR_SMP_00752");

			foreach (SM_ApplicationView app : SM_ClanClientData.Applications)
			{
				int appRow = m_InvitesList.AddItem(SM_PartyLoc.Text(app.Name), NULL, 0);
				m_InvitesList.SetItem(appRow, SM_PartyLoc.Text(app.Date), NULL, 1);
				m_ApplicationRowUids.Insert(app.Uid);
			}

			if (SM_ClanClientData.Applications.Count() == 0)
				m_InvitesHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_00456"));
			else
				m_InvitesHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_00455" + SM_ClanClientData.Applications.Count()));
			return;
		}

		m_HdrInvClan.SetText(SM_PartyLoc.Text("#STR_SMP_00505"));
		m_HdrInvFrom.SetText(SM_PartyLoc.Text("#STR_SMP_00591"));
		SetButtonText(m_AcceptInviteBtn, "#STR_SMP_00844");
		SetButtonText(m_DeclineInviteBtn, "#STR_SMP_00752");

		foreach (SM_InviteView invite : SM_ClanClientData.Invites)
		{
			int row = m_InvitesList.AddItem(SM_PartyLoc.Text(invite.ClanName), NULL, 0);
			m_InvitesList.SetItem(row, SM_PartyLoc.Text(invite.InviterName), NULL, 1);
			m_InviteRowClans.Insert(invite.ClanName);
		}

		if (SM_ClanClientData.Invites.Count() == 0)
			m_InvitesHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_00841"));
		else
			m_InvitesHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_00302" + SM_ClanClientData.Invites.Count()));
	}

	void RefreshPlayers()
	{
		m_PlayersList.ClearItems();
		m_PlayerRowUids.Clear();
		foreach (SM_PlayerEntry entry : SM_ClanClientData.InvitablePlayers)
		{
			m_PlayersList.AddItem(SM_PartyLoc.Text(entry.Name), NULL, 0);
			m_PlayerRowUids.Insert(entry.Uid);
		}
	}


	void OnTopsArrived()
	{
		RefreshTops();
	}

	protected bool IsTopCategoryEnabled(int category)
	{
		if (category < 0 || category >= SM_TopCategory.COUNT)
			return false;
		if (SM_ClanClientData.TopCategoryEnabled.Count() == 0)
			return true;
		if (category >= SM_ClanClientData.TopCategoryEnabled.Count())
			return true;
		return SM_ClanClientData.TopCategoryEnabled[category] != 0;
	}

	protected void NormalizeTopCategory()
	{
		if (IsTopCategoryEnabled(m_TopCategory))
			return;

		for (int category = 0; category < SM_TopCategory.COUNT; category++)
		{
			if (IsTopCategoryEnabled(category))
			{
				m_TopCategory = category;
				return;
			}
		}
	}

	protected bool HasAnyTopCategoryEnabled()
	{
		for (int category = 0; category < SM_TopCategory.COUNT; category++)
		{
			if (IsTopCategoryEnabled(category))
				return true;
		}
		return false;
	}

	protected void UpdateTopsCategoryButtons()
	{
		int visibleIndex = 0;
		for (int u = 0; u < m_TopCatBtns.Count(); u++)
		{
			bool enabled = IsTopCategoryEnabled(u);
			bool active = (u == m_TopCategory);

			if (m_TopCatBgs[u])
				m_TopCatBgs[u].Show(enabled);

			if (m_TopCatBtns[u])
			{
				m_TopCatBtns[u].Show(enabled);
				if (enabled)
				{
					int xpos = visibleIndex * 106;
					m_TopCatBtns[u].SetSize(96, 30);
					m_TopCatBtns[u].SetPos(xpos, 0);
					int catColor = ARGB(210, 196, 180, 157);
					if (active)
						catColor = ARGB(255, 250, 240, 228);
					m_TopCatBtns[u].SetTextColor(catColor);

					if (m_TopCatBgs[u])
					{
						m_TopCatBgs[u].SetSize(96, 30);
						m_TopCatBgs[u].SetPos(xpos, 0);
						int bgColor = ARGB(217, 45, 38, 32);
						if (active)
							bgColor = ARGB(242, 69, 59, 49);
						m_TopCatBgs[u].SetColor(bgColor);
					}
					visibleIndex++;
				}
			}
			if (m_TopCatUnders[u])
			{
				m_TopCatUnders[u].Show(enabled && active);
				if (enabled)
				{
					m_TopCatUnders[u].SetSize(96, 2);
					m_TopCatUnders[u].SetPos((visibleIndex - 1) * 106, 32);
				}
			}
		}
	}

	protected void UpdateTopsHeaderAndHint()
	{
		if (!m_TopsHeaderText || !m_TopsHintText)
			return;

		if (!HasAnyTopCategoryEnabled())
		{
			m_TopsHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_01004"));
			m_TopsHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00245"));
			return;
		}

		string header = "#STR_SMP_01002" + SM_TopCategory.GetName(m_TopCategory);
		if (SM_ClanClientData.TopSeasonSecondsLeft > 0)
			header = header + "#STR_SMP_00006" + SM_PartyUtil.FormatDuration(SM_ClanClientData.TopSeasonSecondsLeft);
		m_TopsHeaderText.SetText(SM_PartyLoc.Text(header));

		if (m_TopCategory == SM_TopCategory.ACCURACY || m_TopCategory == SM_TopCategory.HEADSHOTS)
			m_TopsHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00292" + SM_ClanClientData.MinShotsForAccuracyTop.ToString() + "#STR_SMP_00036"));
		else if (m_TopCategory == SM_TopCategory.KILL_DEATH)
			m_TopsHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00228"));
		else if (m_TopCategory == SM_TopCategory.LONGEST_PLAYER_KILL)
			m_TopsHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00392"));
		else if (m_TopCategory == SM_TopCategory.DISTANCE)
			m_TopsHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00981"));
		else if (m_TopCategory == SM_TopCategory.BEST_LIFE)
			m_TopsHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00896"));
		else
			m_TopsHintText.SetText(SM_PartyLoc.Text(""));
	}

	protected float GetTopSortValue(SM_ClanTopEntry entry, int category)
	{
		SM_ClanStats st = entry.Stats;

		if (category == SM_TopCategory.ACCURACY || category == SM_TopCategory.HEADSHOTS)
		{
			if (st.ShotsFired < SM_ClanClientData.MinShotsForAccuracyTop)
				return -1;
			if (category == SM_TopCategory.ACCURACY)
				return st.HitsLanded * 10000.0 / st.ShotsFired;
			return st.Headshots * 10000.0 / st.ShotsFired;
		}

		if (category == SM_TopCategory.KILL_DEATH)
		{
			if (st.PlayerKills <= 0)
				return -1;
			if (st.PlayerDeaths <= 0)
				return st.PlayerKills * 10000.0;
			return st.PlayerKills * 10000.0 / st.PlayerDeaths;
		}

		return st.GetValue(category);
	}

	protected string FormatKillDeathValue(SM_ClanStats st)
	{
		if (st.PlayerKills <= 0)
			return "0.0  (" + st.PlayerKills.ToString() + "/" + st.PlayerDeaths.ToString() + ")";

		int tenths;
		if (st.PlayerDeaths <= 0)
			tenths = st.PlayerKills * 10;
		else
			tenths = Math.Round(st.PlayerKills * 10.0 / st.PlayerDeaths);

		int whole = tenths / 10;
		int frac = tenths % 10;
		return whole.ToString() + "." + frac.ToString() + "  (" + st.PlayerKills.ToString() + "/" + st.PlayerDeaths.ToString() + ")";
	}

	protected string FormatTopValue(SM_ClanTopEntry entry, int category)
	{
		SM_ClanStats st = entry.Stats;

		if (category == SM_TopCategory.PLAYER_KILLS)
			return st.PlayerKills.ToString();

		if (category == SM_TopCategory.KILL_DEATH)
			return FormatKillDeathValue(st);

		if (category == SM_TopCategory.ZOMBIE_KILLS)
			return st.ZombieKills.ToString();

		if (category == SM_TopCategory.ONLINE_TIME)
			return SM_PartyUtil.FormatDuration(st.OnlineSeconds);

		if (category == SM_TopCategory.BEST_LIFE)
		{
			string life = SM_PartyUtil.FormatDuration(st.BestLifeSeconds);
			if (st.BestLifeName != "")
				life = life + "  (" + st.BestLifeName + ")";
			return life;
		}

		if (category == SM_TopCategory.DISTANCE)
			return SM_PartyUtil.FormatDistance(st.DistanceWalked);

		if (category == SM_TopCategory.LONGEST_PLAYER_KILL)
		{
			if (st.LongestPlayerKillDistance <= 0)
				return "-";
			string longKill = SM_PartyUtil.FormatDistance(st.LongestPlayerKillDistance);
			if (st.LongestPlayerKillName != "")
				longKill = longKill + "  (" + st.LongestPlayerKillName + ")";
			return longKill;
		}

		if (category == SM_TopCategory.ACCURACY)
		{
			if (st.ShotsFired < SM_ClanClientData.MinShotsForAccuracyTop)
				return "#STR_SMP_00017" + st.ShotsFired.ToString() + ")";
			return SM_PartyUtil.FormatPercent(st.HitsLanded, st.ShotsFired) + "  (" + st.HitsLanded.ToString() + "/" + st.ShotsFired.ToString() + ")";
		}

		if (category == SM_TopCategory.HEADSHOTS)
		{
			if (st.ShotsFired < SM_ClanClientData.MinShotsForAccuracyTop)
				return "#STR_SMP_00017" + st.ShotsFired.ToString() + ")";
			return SM_PartyUtil.FormatPercent(st.Headshots, st.ShotsFired) + "  (" + st.Headshots.ToString() + "/" + st.ShotsFired.ToString() + ")";
		}

		return "";
	}

	protected void RefreshTops()
	{
		if (!m_TopsList)
			return;

		NormalizeTopCategory();
		UpdateTopsCategoryButtons();

		if (!HasAnyTopCategoryEnabled())
		{
			m_TopsList.ClearItems();
			UpdateTopsHeaderAndHint();
			return;
		}

		UpdateTopsHeaderAndHint();

		array<int> order = new array<int>;
		for (int i = 0; i < SM_ClanClientData.TopClans.Count(); i++)
		{
			float value = GetTopSortValue(SM_ClanClientData.TopClans[i], m_TopCategory);
			int insertAt = order.Count();
			for (int j = 0; j < order.Count(); j++)
			{
				if (value > GetTopSortValue(SM_ClanClientData.TopClans[order[j]], m_TopCategory))
				{
					insertAt = j;
					break;
				}
			}
			order.InsertAt(i, insertAt);
		}

		m_TopsList.ClearItems();
		for (int p = 0; p < order.Count(); p++)
		{
			SM_ClanTopEntry entry = SM_ClanClientData.TopClans[order[p]];

			string clanTitle = entry.Name;
			if (entry.Tag != "")
				clanTitle = "[" + entry.Tag + "] " + entry.Name;

			int place = p + 1;
			int row = m_TopsList.AddItem(SM_PartyLoc.Text(place.ToString() + "."), NULL, 0);
			m_TopsList.SetItem(row, SM_PartyLoc.Text(clanTitle), NULL, 1);
			m_TopsList.SetItem(row, SM_PartyLoc.Text(FormatTopValue(entry, m_TopCategory)), NULL, 2);

			if (p == 0)
				m_TopsList.SetItemColor(row, 0, ARGB(255, 232, 181, 69));
			else if (p == 1)
				m_TopsList.SetItemColor(row, 0, ARGB(255, 220, 202, 176));
			else if (p == 2)
				m_TopsList.SetItemColor(row, 0, ARGB(255, 177, 113, 53));

			if (SM_ClanClientData.HasClan && entry.Name == SM_ClanClientData.ClanName)
				m_TopsList.SetItemColor(row, 1, SM_ClanClientData.GetClanColor());
		}
	}

	protected int GetMyCurrencyPoints()
	{
		Man player = GetGame().GetPlayer();
		if (!player || SM_ClanClientData.CurrencyItems.Count() == 0)
			return 0;

		array<EntityAI> items = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);

		int points = 0;
		foreach (EntityAI item : items)
		{
			if (!item)
				continue;

			string itemType = item.GetType();
			itemType.ToLower();

			foreach (SM_CurrencyItem currency : SM_ClanClientData.CurrencyItems)
			{
				string classname = currency.Classname;
				classname.ToLower();
				if (classname != itemType)
					continue;

				int count = 1;
				ItemBase ib = ItemBase.Cast(item);
				if (ib && ib.CanBeSplit())
				{
					count = Math.Floor(ib.GetQuantity());
					if (count < 1)
						count = 1;
				}
				points += currency.Value * count;
				break;
			}
		}
		return points;
	}

	protected void RefreshTreasury()
	{
		if (!m_PanelTreasury || !SM_ClanClientData.HasClan)
			return;

		m_TreasuryBalanceText.SetText(SM_PartyLoc.Text("#STR_SMP_00490" + SM_ClanClientData.Treasury.ToString() + "#STR_SMP_00038"));
		if (SM_ClanClientData.IsAdminMode)
			m_TreasuryMyText.SetText(SM_PartyLoc.Text("#STR_SMP_00250"));
		else
			m_TreasuryMyText.SetText(SM_PartyLoc.Text("#STR_SMP_00402" + GetMyCurrencyPoints().ToString()));

		if (SM_ClanClientData.CurrencyItems.Count() == 0)
		{
			m_TreasuryRatesText.SetText(SM_PartyLoc.Text("#STR_SMP_00296"));
		}
		else
		{
			string rates = "";
			for (int i = 0; i < SM_ClanClientData.CurrencyItems.Count(); i++)
			{
				if (i > 0)
					rates = rates + "\n";
				string currencyName = SM_PartyUtil.GetItemDisplayName(SM_ClanClientData.CurrencyItems[i].Classname);
				rates = rates + currencyName + " = " + SM_ClanClientData.CurrencyItems[i].Value.ToString();
			}
			m_TreasuryRatesText.SetText(SM_PartyLoc.Text(rates));
		}
		if (m_TreasuryRatesScroll)
		{
			m_TreasuryRatesScroll.Update();
			m_TreasuryRatesScroll.VScrollToPos01(0.0);
		}

		bool canWithdraw = SM_ClanClientData.MyRank >= SM_ClanClientData.GetActionRank(SM_ClanAction.TREASURY_WITHDRAW);
		bool canUpgrade = SM_ClanClientData.MyRank >= SM_ClanClientData.GetActionRank(SM_ClanAction.UPGRADE);

		bool canDeposit = !SM_ClanClientData.IsAdminMode;
		m_DepositBtn.Show(canDeposit);
		ShowBtnDressing("DepositBtn", canDeposit);
		m_DepositEditBg.Show(canDeposit);
		m_DepositAmountEdit.Show(canDeposit);
		ShowEditUnderline("DepositAmountEdit", canDeposit);
		m_DepositAmountBtn.Show(canDeposit);
		ShowBtnDressing("DepositAmountBtn", canDeposit);

		m_WithdrawLabel.Show(canWithdraw);
		m_WithdrawEditBg.Show(canWithdraw);
		m_WithdrawAmountEdit.Show(canWithdraw);
		ShowEditUnderline("WithdrawAmountEdit", canWithdraw);
		m_WithdrawBtn.Show(canWithdraw);
		ShowBtnDressing("WithdrawBtn", canWithdraw);

		if (SM_ClanClientData.ClanLevelsEnabled)
		{
			m_LevelCardBg.Show(true);
			m_LevelHeader.Show(true);
			m_LevelHeaderLine.Show(true);
			m_LevelInfoText.Show(true);
			if (m_LevelLadderScroll)
				m_LevelLadderScroll.Show(true);
			if (m_LevelColSep)
				m_LevelColSep.Show(true);

			int level = SM_ClanClientData.ClanLevel;
			int maxLevel = SM_ClanClientData.GetMaxLevel();
			int cost = SM_ClanClientData.GetNextUpgradeCost();

			// Полная лестница: показываем все уровни сразу, чтобы заранее
			// понимать, что даёт каждый и сколько стоит дальнейшая прокачка.
			string ladder = "";
			for (int lvl = 1; lvl <= maxLevel; lvl++)
			{
				SM_ClanLevel data = SM_ClanClientData.GetLevelData(lvl);
				if (!data)
					continue;

				if (ladder != "")
					ladder = ladder + "\n";

				string line = "#STR_SMP_01040" + lvl.ToString() + "#STR_SMP_00012" + data.StorageSlots.ToString() + "#STR_SMP_00157" + data.MarketLots.ToString() + "#STR_SMP_00160" + data.ServerMarketItems.ToString();

				if (lvl < level)
					line = line + "#STR_SMP_00002";
				else if (lvl == level)
					line = line + "#STR_SMP_00003";
				else if (lvl == level + 1)
					line = line + "#STR_SMP_00001" + data.UpgradeCost.ToString() + "#STR_SMP_00052";
				else
					line = line + "   — " + data.UpgradeCost.ToString() + "#STR_SMP_00052";

				if (lvl == maxLevel && maxLevel > 1)
					line = line + "#STR_SMP_00018";

				ladder = ladder + line;
			}
			if (m_LevelLadderText)
				m_LevelLadderText.SetText(SM_PartyLoc.Text(ladder));
			if (m_LevelLadderScroll)
			{
				m_LevelLadderScroll.Update();
				m_LevelLadderScroll.VScrollToPos01(0.0);
			}

			// Краткая сводка справа (2 строки): подробности по уровням — в «лестнице» слева.
			string li = "#STR_SMP_00989" + level.ToString() + " / " + maxLevel.ToString();

			if (cost < 0)
			{
				li = li + "#STR_SMP_00108";
				m_UpgradeClanBtn.Show(false);
				ShowBtnDressing("UpgradeClanBtn", false);
				m_UpgradeProgressTrack.Show(false);
				m_UpgradeProgressText.Show(false);
			}
			else
			{
				int nextLevel = level + 1;
				SM_ClanLevel next = SM_ClanClientData.GetLevelData(nextLevel);
				if (next)
					li = li + "#STR_SMP_00116" + nextLevel.ToString() + "#STR_SMP_00023" + next.StorageSlots.ToString() + "#STR_SMP_00157" + next.MarketLots.ToString() + "#STR_SMP_00160" + next.ServerMarketItems.ToString() + ")";

				float frac = 1.0;
				if (cost > 0)
					frac = SM_ClanClientData.Treasury / (float)cost;
				frac = Math.Clamp(frac, 0.0, 1.0);
				m_UpgradeProgressFill.SetSize(frac, 1.0);

				int pct = (int)Math.Round(frac * 100.0);
				m_UpgradeProgressText.SetText(SM_PartyLoc.Text("#STR_SMP_00493" + SM_ClanClientData.Treasury.ToString() + " / " + cost.ToString() + " (" + pct.ToString() + "%)"));
				m_UpgradeProgressTrack.Show(true);
				m_UpgradeProgressText.Show(true);

				m_UpgradeClanBtn.Show(canUpgrade);
				ShowBtnDressing("UpgradeClanBtn", canUpgrade);
				if (canUpgrade)
				{
					if (SM_ClanClientData.Treasury >= cost)
						SetButtonText(m_UpgradeClanBtn, "#STR_SMP_00873" + nextLevel.ToString() + " (-" + cost.ToString() + ")");
					else
						SetButtonText(m_UpgradeClanBtn, "#STR_SMP_00413" + nextLevel.ToString() + "#STR_SMP_00063" + cost.ToString() + "#STR_SMP_00026");
				}
			}
			m_LevelInfoText.SetText(SM_PartyLoc.Text(li));
		}
		else
		{
			m_LevelCardBg.Show(false);
			m_LevelHeader.Show(false);
			m_LevelHeaderLine.Show(false);
			m_LevelInfoText.Show(false);
			if (m_LevelLadderScroll)
				m_LevelLadderScroll.Show(false);
			if (m_LevelColSep)
				m_LevelColSep.Show(false);
			m_UpgradeClanBtn.Show(false);
			ShowBtnDressing("UpgradeClanBtn", false);
			m_UpgradeProgressTrack.Show(false);
			m_UpgradeProgressText.Show(false);
		}

		if (SM_ClanClientData.IsAdminMode)
			m_TreasuryHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00249"));
		else
			m_TreasuryHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00220"));
	}

	protected void RefreshLog()
	{
		m_LogList.ClearItems();
		foreach (string line : SM_ClanClientData.LogEntries)
			m_LogList.AddItem(SM_PartyLoc.Text(line), NULL, 0);
	}

	protected void RefreshAdminChatLog()
	{
		m_AdminChatRowUids.Clear();
		m_AdminChatRowNames.Clear();

		if (!m_AdminChatList)
			return;

		m_AdminChatList.ClearItems();

		foreach (SM_AdminChatLogEntry entry : SM_ClanClientData.AdminChatLog)
		{
			if (!entry)
				continue;

			int row = m_AdminChatList.AddItem(SM_PartyLoc.Text(entry.Stamp), NULL, 0);
			m_AdminChatList.SetItem(row, SM_PartyLoc.Text(entry.Channel), NULL, 1);
			m_AdminChatList.SetItem(row, SM_PartyLoc.Text(entry.Name), NULL, 2);
			m_AdminChatList.SetItem(row, SM_PartyLoc.Text(entry.Text), NULL, 3);
			m_AdminChatRowUids.Insert(entry.Uid);
			m_AdminChatRowNames.Insert(entry.Name);
		}
	}

	protected void RefreshOnlineWidget()
	{
		int count = SM_ClanClientData.OnlinePlayers.Count();
		if (m_OnlineValueText)
			m_OnlineValueText.SetText(SM_PartyLoc.Text(count.ToString()));
		if (m_OnlineHeaderText)
			m_OnlineHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_00472" + count.ToString()));

		if (!m_OnlineList)
			return;

		m_OnlineList.ClearItems();
		foreach (SM_OnlinePlayerView view : SM_ClanClientData.OnlinePlayers)
		{
			if (!view)
				continue;

			string clanName = view.ClanName;
			if (clanName == "")
				clanName = "-";

			string title = view.Title;
			if (title == "")
				title = "-";

			int row = m_OnlineList.AddItem(SM_PartyLoc.Text(view.Name), NULL, 0);
			m_OnlineList.SetItem(row, SM_PartyLoc.Text(clanName), NULL, 1);
			m_OnlineList.SetItem(row, SM_PartyLoc.Text(title), NULL, 2);
			if (view.TitleColor != 0)
				m_OnlineList.SetItemColor(row, 2, view.TitleColor);
		}
	}

	protected bool IsOnlinePanelOpen()
	{
		if (!m_PanelOnline)
			return false;
		return m_PanelOnline.IsVisible();
	}

	protected void ShowOnlinePanel()
	{
		if (m_PanelOnline)
			m_PanelOnline.Show(true);
		RefreshOnlineWidget();
		RequestOnlinePlayers();
	}

	protected void HideOnlinePanel()
	{
		if (m_PanelOnline)
			m_PanelOnline.Show(false);
	}

	protected void ToggleOnlinePanel()
	{
		if (IsOnlinePanelOpen())
		{
			HideOnlinePanel();
			return;
		}

		ShowOnlinePanel();
	}

	protected string GetSelectedAdminChatUid()
	{
		if (!m_AdminChatList)
			return "";
		int row = m_AdminChatList.GetSelectedRow();
		if (row >= 0 && row < m_AdminChatRowUids.Count())
			return m_AdminChatRowUids[row];
		return "";
	}

	protected string GetSelectedAdminChatName()
	{
		if (!m_AdminChatList)
			return "";
		int row = m_AdminChatList.GetSelectedRow();
		if (row >= 0 && row < m_AdminChatRowNames.Count())
			return m_AdminChatRowNames[row];
		return "";
	}

	protected void ShowAdminChatMutePanel()
	{
		string uid = GetSelectedAdminChatUid();
		string name = GetSelectedAdminChatName();
		if (uid == "")
			return;

		m_AdminChatMuteUid = uid;
		m_AdminChatMuteName = name;

		if (m_AdminChatMuteTargetText)
			m_AdminChatMuteTargetText.SetText(SM_PartyLoc.Text("#STR_SMP_00469" + name));
		if (m_AdminChatMuteMinutesEdit)
			m_AdminChatMuteMinutesEdit.SetText(SM_PartyLoc.Text("30"));
		if (m_AdminChatMutePanel)
			m_AdminChatMutePanel.Show(true);
	}

	protected void HideAdminChatMutePanel()
	{
		if (m_AdminChatMutePanel)
			m_AdminChatMutePanel.Show(false);
	}

	protected int GetAdminChatMuteMinutes()
	{
		if (!m_AdminChatMuteMinutesEdit)
			return 0;

		string value = m_AdminChatMuteMinutesEdit.GetText();
		value = value.Trim();
		int minutes = value.ToInt();
		if (minutes < 0)
			minutes = 0;
		return minutes;
	}

	protected void RequestAdminChatLog()
	{
		if (!SM_ClanClientData.IsAdmin)
			return;
		SendSimpleRpc(SM_PartyRPC.REQUEST_ADMIN_CHAT_LOG);
	}

	protected void RequestOnlinePlayers()
	{
		SendSimpleRpc(SM_PartyRPC.REQUEST_ONLINE_PLAYERS);
	}

	protected void SendAdminChatMuteFromPanel()
	{
		if (m_AdminChatMuteUid == "")
			return;

		int minutes = GetAdminChatMuteMinutes();
		if (minutes <= 0)
			return;

		Man player = GetGame().GetPlayer();
		if (!player)
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_AdminChatMuteUid);
		rpc.Write(minutes);
		rpc.Write("#STR_SMP_00243");
		rpc.Send(player, SM_PartyRPC.ADMIN_CHAT_MUTE, true, NULL);

		HideAdminChatMutePanel();
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.RequestAdminChatLog, 250, false);
	}

	protected void SendAdminChatUnmuteFromPanel()
	{
		if (m_AdminChatMuteUid == "")
			return;

		Man player = GetGame().GetPlayer();
		if (!player)
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(m_AdminChatMuteUid);
		rpc.Send(player, SM_PartyRPC.ADMIN_CHAT_UNMUTE, true, NULL);

		HideAdminChatMutePanel();
		GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(this.RequestAdminChatLog, 250, false);
	}

	protected void RefreshDescPanel()
	{
		if (!SM_ClanClientData.HasClan)
			return;

		bool canDesc = SM_ClanClientData.MyRank >= SM_ClanClientData.GetActionRank(SM_ClanAction.DESCRIPTION);

		m_DescEdit.Show(canDesc);
		ShowEditUnderline("DescEdit", canDesc);
		m_SaveDescBtn.Show(canDesc);
		ShowBtnDressing("SaveDescBtn", canDesc);
		m_DescViewText.Show(!canDesc);

		string desc = SM_ClanClientData.ClanDescription;
		if (desc == "")
			desc = "#STR_SMP_00733";

		if (canDesc)
		{
			m_DescEdit.SetText(SM_PartyLoc.Text(SM_ClanClientData.ClanDescription));
			m_DescHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00730"));
		}
		else
		{
			m_DescViewText.SetText(SM_PartyLoc.Text(desc));
			m_DescHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00636"));
		}
	}

	protected bool CanManagePerms()
	{
		if (!SM_ClanClientData.HasClan)
			return false;
		if (SM_ClanClientData.IsAdminMode)
			return true;
		if (SM_ClanClientData.MyRank == SM_ClanClientData.GetLeaderRank())
			return true;
		return false;
	}

	protected bool CanManageWebhook()
	{
		if (!SM_ClanClientData.HasClan)
			return false;
		if (SM_ClanClientData.IsAdminMode)
			return true;
		if (SM_ClanClientData.MyRank >= SM_ClanClientData.GetActionRank(SM_ClanAction.MANAGE_WEBHOOK))
			return true;
		return false;
	}

	protected bool CanOpenPermsPanel()
	{
		if (CanManagePerms())
			return true;
		if (CanManageWebhook())
			return true;
		return false;
	}

	protected void RefreshPerms()
	{
		if (!m_PanelPerms || !SM_ClanClientData.HasClan)
			return;

		bool canManagePerms = CanManagePerms();
		bool canManageWebhook = CanManageWebhook();

		int prevRow = m_PermsList.GetSelectedRow();

		m_PermsList.ClearItems();
		m_PermRowActions.Clear();

		for (int action = 0; action < SM_ClanAction.COUNT; action++)
		{
			if (action == SM_ClanAction.MARKET_SELL && !SM_ClanClientData.MarketEnabled)
				continue;
			if (action == SM_ClanAction.TREASURY_WITHDRAW && !SM_ClanClientData.TreasuryEnabled)
				continue;
			if (action == SM_ClanAction.STORAGE_TAKE && !SM_ClanClientData.StorageEnabled)
				continue;
			if (action == SM_ClanAction.UPGRADE && !SM_ClanClientData.ClanLevelsEnabled)
				continue;

			int minRank = SM_ClanClientData.GetActionRank(action);
			int row = m_PermsList.AddItem(SM_PartyLoc.Text(SM_ClanAction.GetName(action)), NULL, 0);
			m_PermsList.SetItem(row, SM_PartyLoc.Text(SM_ClanClientData.GetRankName(minRank) + "#STR_SMP_00043"), NULL, 1);
			m_PermRowActions.Insert(action);
		}

		if (prevRow >= 0 && prevRow < m_PermRowActions.Count())
			m_PermsList.SelectRow(prevRow);
		else if (m_PermRowActions.Count() > 0)
			m_PermsList.SelectRow(0);

		m_PermRankUpBtn.Show(canManagePerms);
		ShowBtnDressing("PermRankUpBtn", canManagePerms);
		m_PermRankDownBtn.Show(canManagePerms);
		ShowBtnDressing("PermRankDownBtn", canManagePerms);

		if (canManagePerms)
			m_PermsHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00353"));
		else
			m_PermsHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00824"));

		m_WebhookStatusText.Show(canManageWebhook);
		m_WebhookEditBg.Show(canManageWebhook);
		m_WebhookEdit.Show(canManageWebhook);
		ShowEditUnderline("WebhookEdit", canManageWebhook);
		m_WebhookSaveBtn.Show(canManageWebhook);
		ShowBtnDressing("WebhookSaveBtn", canManageWebhook);
		m_WebhookClearBtn.Show(canManageWebhook);
		ShowBtnDressing("WebhookClearBtn", canManageWebhook);
		m_WebhookTestBtn.Show(canManageWebhook);
		ShowBtnDressing("WebhookTestBtn", canManageWebhook);
		m_WebhookAuditBtn.Show(canManageWebhook);
		ShowBtnDressing("WebhookAuditBtn", canManageWebhook);

		if (canManageWebhook)
		{
			string webhookStatus = "#STR_SMP_00239";
			if (SM_ClanClientData.ClanWebhookConfigured)
			{
				if (SM_ClanClientData.ClanWebhookEnabled)
					webhookStatus = "#STR_SMP_00237";
				else
					webhookStatus = "#STR_SMP_00238";
			}
			if (SM_ClanClientData.ClanWebhookAuditEnabled)
				webhookStatus = webhookStatus + "#STR_SMP_00154";
			m_WebhookStatusText.SetText(SM_PartyLoc.Text(webhookStatus));

			if (SM_ClanClientData.ClanWebhookAuditEnabled)
				m_WebhookAuditBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00376"));
			else
				m_WebhookAuditBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00323"));

			if (SM_ClanClientData.ClanWebhookConfigured && SM_ClanClientData.ClanWebhookEnabled)
				m_WebhookAuditBtn.Enable(true);
			else
				m_WebhookAuditBtn.Enable(false);
		}
	}

	protected void ChangeSelectedPerm(int delta)
	{
		if (!CanManagePerms())
			return;

		int row = m_PermsList.GetSelectedRow();
		if (row < 0 || row >= m_PermRowActions.Count())
			return;

		int action = m_PermRowActions[row];
		int newRank = SM_ClanClientData.GetActionRank(action) + delta;
		int leaderRank = SM_ClanClientData.GetLeaderRank();
		if (newRank < 0)
			newRank = 0;
		if (newRank > leaderRank)
			newRank = leaderRank;

		Man player = GetGame().GetPlayer();
		if (!player)
			return;

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(action);
		rpc.Write(newRank);
		rpc.Send(player, SM_PartyRPC.SET_PERMISSION, true, NULL);
	}

	void OnClanInfoArrived()
	{
		if (m_CurrentTab != 1)
			return;

		m_InfoMode = true;
		ShowTab();
		RefreshClanInfo();
	}

	protected void RefreshClanInfo()
	{
		int infoColor = SM_ClanColors.ResolveColor(SM_ClanClientData.InfoColorIndex);

		string title = SM_ClanClientData.InfoName;
		if (SM_ClanClientData.InfoTag != "")
			title = "[" + SM_ClanClientData.InfoTag + "] " + title;
		m_InfoTitleText.SetText(SM_PartyLoc.Text(title));
		m_InfoTitleText.SetColor(infoColor);

		string desc = SM_ClanClientData.InfoDescription;
		if (desc == "")
			desc = "#STR_SMP_00733";
		m_InfoDescText.SetText(SM_PartyLoc.Text(desc));

		m_InfoMembersList.ClearItems();
		array<SM_ClanMemberView> orderedInfoMembers = OrderMembersOnlineFirst(SM_ClanClientData.InfoMembers);
		foreach (SM_ClanMemberView member : orderedInfoMembers)
		{
			string status = "Offline";
			if (member.Online)
				status = "Online";

			int row = m_InfoMembersList.AddItem(SM_PartyLoc.Text(member.Name), NULL, 0);
			m_InfoMembersList.SetItem(row, SM_PartyLoc.Text(SM_ClanClientData.GetRankName(member.Rank)), NULL, 1);
			string playerTitle = member.PlayerTitle;
			if (playerTitle == "")
				playerTitle = "-";
			m_InfoMembersList.SetItem(row, SM_PartyLoc.Text(playerTitle), NULL, 2);
			m_InfoMembersList.SetItem(row, SM_PartyLoc.Text(status), NULL, 3);

			if (!member.Online)
			{
				m_InfoMembersList.SetItemColor(row, 0, ARGB(255, 143, 131, 114));
				m_InfoMembersList.SetItemColor(row, 1, ARGB(255, 143, 131, 114));
				m_InfoMembersList.SetItemColor(row, 2, ARGB(255, 143, 131, 114));
				m_InfoMembersList.SetItemColor(row, 3, ARGB(255, 143, 131, 114));
			}
			else if (member.PlayerTitleColor != 0)
			{
				m_InfoMembersList.SetItemColor(row, 2, member.PlayerTitleColor);
			}
		}

		m_ApplyBtn.Show(!SM_ClanClientData.HasClan);
		ShowBtnDressing("ApplyBtn", !SM_ClanClientData.HasClan);
		bool canAdminEnter = false;
		if (SM_ClanClientData.IsAdmin && SM_ClanClientData.InfoName != "")
		{
			canAdminEnter = true;
			if (SM_ClanClientData.IsAdminMode && SM_ClanClientData.InfoName == SM_ClanClientData.ClanName)
				canAdminEnter = false;
		}
		m_AdminEnterClanBtn.Show(canAdminEnter);
		ShowBtnDressing("AdminEnterClanBtn", canAdminEnter);
	}

	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		if (w == m_ClansList)
		{
			int row = m_ClansList.GetSelectedRow();
			if (row >= 0 && row < m_ClanRowNames.Count())
				SendStringRpc(SM_PartyRPC.REQUEST_CLAN_INFO, m_ClanRowNames[row]);
			return true;
		}

		if (w == m_ContractTargetsList)
		{
			string targetUid = GetSelectedContractTargetUid();
			if (targetUid != "")
			{
				ClearContractTargetPreview();
				SendContractTargetPreviewRequest(targetUid);
			}
			else if (m_ContractInfoText)
			{
				m_ContractInfoText.SetText(SM_PartyLoc.Text("#STR_SMP_00365"));
			}
			return true;
		}

		if (w == m_AdminChatList)
		{
			ShowAdminChatMutePanel();
			return true;
		}

		if (w == m_ClanMap)
		{
			if (button == 0)
			{
				NormalizeMapMarkerMode();
				if (!IsMapMarkerModeEditable())
				{
					if (m_MapSelectedText)
						m_MapSelectedText.SetText(SM_PartyLoc.Text(""));
					return true;
				}

				vector mapPos = m_ClanMap.ScreenToMap(Vector(x, y, 0));
				mapPos[1] = GetGame().SurfaceY(mapPos[0], mapPos[2]);
				m_SelectedMapPosition = mapPos;
				m_HasSelectedMapPosition = true;

				if (m_MapMarkerNameEdit)
				{
					string markerName = m_MapMarkerNameEdit.GetText();
					markerName = markerName.Trim();
					if (markerName == "")
						m_MapMarkerNameEdit.SetText(SM_PartyLoc.Text("#STR_SMP_00639"));
				}

				RefreshMapMarks();
				RefreshMapMarkerList();
				OpenMapMarkerCreateEditor(mapPos);
			}
			return true;
		}

		return super.OnDoubleClick(w, x, y, button);
	}

	// Модальное окно подтверждения роспуска в стиле мода (вместо нативного диалога DayZ).
	protected void ShowDisbandConfirm()
	{
		if (m_DisbandConfirmText)
		{
			string msg = "#STR_SMP_00509";
			if (SM_ClanClientData.Treasury > 0)
				msg = msg + "#STR_SMP_00404" + SM_ClanClientData.Treasury.ToString() + "#STR_SMP_00140";
			msg = msg + "#STR_SMP_00922";
			m_DisbandConfirmText.SetText(SM_PartyLoc.Text(msg));
		}

		if (m_DisbandConfirmOverlay)
			m_DisbandConfirmOverlay.Show(true);
	}

	protected void HideDisbandConfirm()
	{
		if (m_DisbandConfirmOverlay)
			m_DisbandConfirmOverlay.Show(false);
	}

	protected bool IsDisbandConfirmOpen()
	{
		return m_DisbandConfirmOverlay && m_DisbandConfirmOverlay.IsVisible();
	}

	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w == m_MapMarkerEditCloseBtn)
		{
			HideMapMarkerEditor();
			return true;
		}

		if (w == m_MapMarkerEditColorPrevBtn)
		{
			m_MapMarkerEditColor--;
			if (m_MapMarkerEditColor < 0)
				m_MapMarkerEditColor = SM_ClanColors.Count() - 1;
			RefreshMapMarkerEditorColor();
			if (m_MapMarkerEditNew)
			{
				m_SelectedMarkerColor = m_MapMarkerEditColor;
				RefreshMapMarks();
			}
			return true;
		}

		if (w == m_MapMarkerEditColorNextBtn)
		{
			m_MapMarkerEditColor++;
			if (m_MapMarkerEditColor >= SM_ClanColors.Count())
				m_MapMarkerEditColor = 0;
			RefreshMapMarkerEditorColor();
			if (m_MapMarkerEditNew)
			{
				m_SelectedMarkerColor = m_MapMarkerEditColor;
				RefreshMapMarks();
			}
			return true;
		}

		if (w == m_MapMarkerEditIconPrevBtn)
		{
			m_MapMarkerEditIcon--;
			if (m_MapMarkerEditIcon < 0)
				m_MapMarkerEditIcon = SM_MapMarkerIconSet.Count() - 1;
			UpdateMapMarkerEditorIcon();
			if (m_MapMarkerEditNew)
			{
				m_SelectedMarkerIcon = m_MapMarkerEditIcon;
				RefreshMapMarks();
			}
			return true;
		}

		if (w == m_MapMarkerEditIconNextBtn)
		{
			m_MapMarkerEditIcon++;
			if (m_MapMarkerEditIcon >= SM_MapMarkerIconSet.Count())
				m_MapMarkerEditIcon = 0;
			UpdateMapMarkerEditorIcon();
			if (m_MapMarkerEditNew)
			{
				m_SelectedMarkerIcon = m_MapMarkerEditIcon;
				RefreshMapMarks();
			}
			return true;
		}

		if (w == m_MapMarkerEditSaveBtn)
		{
			SaveMapMarkerEditor();
			return true;
		}

		if (w == m_MapMarkerEditDeleteBtn)
		{
			DeleteMapMarkerEditor();
			return true;
		}

		int mapMarkerWidgetIndex = FindMapMarkerWidgetButtonIndex(w);
		if (mapMarkerWidgetIndex >= 0)
		{
			OpenMapMarkerEditorByWidgetIndex(mapMarkerWidgetIndex);
			return true;
		}

		if (w == m_CloseBtn)
		{
			Close();
			return true;
		}

		if (w == m_OnlineBtn)
		{
			ToggleOnlinePanel();
			return true;
		}

		if (w == m_OnlineCloseBtn)
		{
			HideOnlinePanel();
			return true;
		}

		if (w == m_ContractTargetPreviewCloseBtn)
		{
			ClearContractTargetPreview();
			return true;
		}

		int marketTileIndex = FindItemTileIndex(w, m_MarketTileWidgets);
		if (marketTileIndex >= 0)
		{
			m_MarketList.SelectRow(marketTileIndex);
			UpdateItemTileSelection(m_MarketTileWidgets, marketTileIndex);
			UpdateMarketInfo();
			return true;
		}

		int serverMarketTileIndex = FindItemTileIndex(w, m_ServerMarketTileWidgets);
		if (serverMarketTileIndex >= 0)
		{
			m_ServerMarketList.SelectRow(serverMarketTileIndex);
			UpdateItemTileSelection(m_ServerMarketTileWidgets, serverMarketTileIndex);
			UpdateServerMarketInfo();
			return true;
		}

		int auctionTileIndex = FindItemTileIndex(w, m_AuctionTileWidgets);
		if (auctionTileIndex >= 0)
		{
			m_AuctionList.SelectRow(auctionTileIndex);
			UpdateItemTileSelection(m_AuctionTileWidgets, auctionTileIndex);
			UpdateAuctionInfo();
			return true;
		}

		int storageTileIndex = FindItemTileIndex(w, m_StorageTileWidgets);
		if (storageTileIndex >= 0)
		{
			m_StorageList.SelectRow(storageTileIndex);
			UpdateItemTileSelection(m_StorageTileWidgets, storageTileIndex);
			UpdateStorageInfo();
			return true;
		}

		if (w == m_TabMyClanBtn)
		{
			m_CurrentTab = 0;
			m_InviteMode = false;
			m_DescMode = false;
			ShowTab();
			return true;
		}

		if (w == m_TabClansBtn)
		{
			m_CurrentTab = 1;
			m_InfoMode = false;
			ShowTab();
			SendSimpleRpc(SM_PartyRPC.REQUEST_CLAN_LIST);
			return true;
		}

		if (w == m_TabInvitesBtn)
		{
			m_CurrentTab = 2;
			ShowTab();
			return true;
		}

		if (w == m_TabTopsBtn)
		{
			m_CurrentTab = 3;
			ShowTab();
			RefreshTops();
			SendSimpleRpc(SM_PartyRPC.REQUEST_TOPS);
			return true;
		}

		if (w == m_TabTreasuryBtn)
		{
			m_CurrentTab = 4;
			ShowTab();
			RefreshTreasury();
			return true;
		}

		if (w == m_TabLogBtn)
		{
			m_CurrentTab = 5;
			ShowTab();
			return true;
		}

		if (w == m_TabSettingsBtn)
		{
			m_CurrentTab = 12;
			ShowTab();
			return true;
		}

		if (w == m_TabContractsBtn)
		{
			m_CurrentTab = 14;
			ShowTab();
			RequestContracts();
			return true;
		}

		if (w == m_TabAdminChatBtn)
		{
			m_CurrentTab = 15;
			ShowTab();
			RequestAdminChatLog();
			return true;
		}

		if (w == m_AdminChatRefreshBtn)
		{
			RequestAdminChatLog();
			return true;
		}

		if (w == m_AdminChatMuteCloseBtn)
		{
			HideAdminChatMutePanel();
			return true;
		}

		if (w == m_AdminChatMuteApplyBtn)
		{
			SendAdminChatMuteFromPanel();
			return true;
		}

		if (w == m_AdminChatUnmuteBtn)
		{
			SendAdminChatUnmuteFromPanel();
			return true;
		}

		for (int si = 0; si < m_SettingDecBtns.Count(); si++)
		{
			if (w == m_SettingDecBtns[si])
			{
				AdjustSetting(si, -1);
				SetFocus(NULL);
				return true;
			}
			if (si < m_SettingIncBtns.Count() && w == m_SettingIncBtns[si])
			{
				AdjustSetting(si, 1);
				SetFocus(NULL);
				return true;
			}
		}

		for (int ti = 0; ti < m_SettingToggles.Count(); ti++)
		{
			if (w == m_SettingToggles[ti])
			{
				ToggleSetting(ti);
				SetFocus(NULL);
				return true;
			}
		}

		if (w == m_MembersCloseBtn)
		{
			CloseMembersOverlay();
			return true;
		}

		for (int mi = 0; mi < m_MemberRowDec.Count(); mi++)
		{
			if (w == m_MemberRowDec[mi])
			{
				if (mi < m_HudSelUids.Count() && m_HudSelUids[mi] != "")
					SM_ClanClientData.SetHudMemberHidden(m_HudSelUids[mi], true);
				RefreshMemberOverlay();
				SetFocus(NULL);
				return true;
			}
			if (mi < m_MemberRowInc.Count() && w == m_MemberRowInc[mi])
			{
				if (mi < m_HudSelUids.Count() && m_HudSelUids[mi] != "")
					SM_ClanClientData.SetHudMemberHidden(m_HudSelUids[mi], false);
				RefreshMemberOverlay();
				SetFocus(NULL);
				return true;
			}
		}

		if (m_TabMapBtn && w == m_TabMapBtn)
		{
			return true;
		}

		for (int t = 0; t < m_TopCatBtns.Count(); t++)
		{
			if (w == m_TopCatBtns[t])
			{
				if (!IsTopCategoryEnabled(t))
					return true;
				m_TopCategory = t;
				RefreshTops();
				SetFocus(NULL);
				return true;
			}
		}

		if (w == m_DepositBtn)
		{
			SendIntRpc(SM_PartyRPC.TREASURY_DEPOSIT, 0);
			return true;
		}

		if (w == m_DepositAmountBtn)
		{
			string depositText = m_DepositAmountEdit.GetText();
			depositText = depositText.Trim();
			int depositAmount = depositText.ToInt();
			if (depositAmount > 0)
			{
				SendIntRpc(SM_PartyRPC.TREASURY_DEPOSIT, depositAmount);
				m_DepositAmountEdit.SetText(SM_PartyLoc.Text(""));
			}
			return true;
		}

		if (w == m_UpgradeClanBtn)
		{
			SendSimpleRpc(SM_PartyRPC.CLAN_UPGRADE);
			return true;
		}

		if (w == m_WithdrawBtn)
		{
			string amountText = m_WithdrawAmountEdit.GetText();
			amountText = amountText.Trim();
			int amount = amountText.ToInt();
			if (amount > 0)
			{
				SendIntRpc(SM_PartyRPC.TREASURY_WITHDRAW, amount);
				m_WithdrawAmountEdit.SetText(SM_PartyLoc.Text(""));
			}
			return true;
		}

		if (w == m_CreateClanBtn)
		{
			OnCreateClicked();
			return true;
		}

		if (w == m_InviteBtn)
		{
			m_InviteMode = true;
			ShowTab();
			SendSimpleRpc(SM_PartyRPC.REQUEST_PLAYERS);
			return true;
		}

		if (w == m_CancelInviteBtn)
		{
			m_InviteMode = false;
			ShowTab();
			return true;
		}

		if (w == m_RefreshPlayersBtn)
		{
			SendSimpleRpc(SM_PartyRPC.REQUEST_PLAYERS);
			return true;
		}

		if (w == m_ConfirmInviteBtn)
		{
			int playerRow = m_PlayersList.GetSelectedRow();
			if (playerRow >= 0 && playerRow < m_PlayerRowUids.Count())
			{
				SendStringRpc(SM_PartyRPC.INVITE, m_PlayerRowUids[playerRow]);
				m_InviteMode = false;
				ShowTab();
			}
			return true;
		}

		if (w == m_KickBtn)
		{
			string kickUid = GetSelectedMemberUid();
			if (kickUid != "" && kickUid != SM_ClanClientData.MyUid)
				SendStringRpc(SM_PartyRPC.KICK, kickUid);
			return true;
		}

		if (w == m_PromoteBtn)
		{
			string promoteUid = GetSelectedMemberUid();
			if (promoteUid != "" && promoteUid != SM_ClanClientData.MyUid)
				SendStringRpc(SM_PartyRPC.PROMOTE, promoteUid);
			return true;
		}

		if (w == m_DemoteBtn)
		{
			string demoteUid = GetSelectedMemberUid();
			if (demoteUid != "" && demoteUid != SM_ClanClientData.MyUid)
				SendStringRpc(SM_PartyRPC.DEMOTE, demoteUid);
			return true;
		}

		if (w == m_LeaveBtn)
		{
			SendSimpleRpc(SM_PartyRPC.LEAVE);
			return true;
		}

		if (w == m_DisbandBtn)
		{
			ShowDisbandConfirm();
			return true;
		}

		if (w == m_DisbandConfirmYesBtn)
		{
			HideDisbandConfirm();
			SendSimpleRpc(SM_PartyRPC.DISBAND);
			return true;
		}

		if (w == m_DisbandConfirmNoBtn)
		{
			HideDisbandConfirm();
			return true;
		}

		if (w == m_AdminLeaveClanBtn)
		{
			SendSimpleRpc(SM_PartyRPC.ADMIN_LEAVE_CLAN);
			return true;
		}

		if (w == m_RefreshClansBtn)
		{
			SendSimpleRpc(SM_PartyRPC.REQUEST_CLAN_LIST);
			return true;
		}

		if (w == m_AcceptInviteBtn)
		{
			int invRow = m_InvitesList.GetSelectedRow();
			if (SM_ClanClientData.HasClan)
			{
				if (invRow >= 0 && invRow < m_ApplicationRowUids.Count())
					SendStringRpc(SM_PartyRPC.APPLICATION_ACCEPT, m_ApplicationRowUids[invRow]);
			}
			else if (invRow >= 0 && invRow < m_InviteRowClans.Count())
			{
				SendStringRpc(SM_PartyRPC.ACCEPT_INVITE, m_InviteRowClans[invRow]);
			}
			return true;
		}

		if (w == m_DeclineInviteBtn)
		{
			int decRow = m_InvitesList.GetSelectedRow();
			if (SM_ClanClientData.HasClan)
			{
				if (decRow >= 0 && decRow < m_ApplicationRowUids.Count())
					SendStringRpc(SM_PartyRPC.APPLICATION_DECLINE, m_ApplicationRowUids[decRow]);
			}
			else if (decRow >= 0 && decRow < m_InviteRowClans.Count())
			{
				SendStringRpc(SM_PartyRPC.DECLINE_INVITE, m_InviteRowClans[decRow]);
			}
			return true;
		}

		if (w == m_DescBtn)
		{
			m_DescMode = true;
			ShowTab();
			RefreshDescPanel();
			return true;
		}

		if (w == m_PermsBtn)
		{
			m_PermsMode = true;
			ShowTab();
			RefreshPerms();
			return true;
		}

		if (w == m_PermsCloseBtn)
		{
			m_PermsMode = false;
			ShowTab();
			return true;
		}

		if (w == m_PermRankUpBtn)
		{
			ChangeSelectedPerm(1);
			return true;
		}

		if (w == m_PermRankDownBtn)
		{
			ChangeSelectedPerm(-1);
			return true;
		}

		if (w == m_WebhookSaveBtn)
		{
			string webhookUrl = m_WebhookEdit.GetText();
			webhookUrl = webhookUrl.Trim();
			if (webhookUrl != "")
			{
				SendStringRpc(SM_PartyRPC.SET_CLAN_WEBHOOK, webhookUrl);
				m_WebhookEdit.SetText(SM_PartyLoc.Text(""));
			}
			return true;
		}

		if (w == m_WebhookClearBtn)
		{
			SendSimpleRpc(SM_PartyRPC.CLEAR_CLAN_WEBHOOK);
			m_WebhookEdit.SetText(SM_PartyLoc.Text(""));
			return true;
		}

		if (w == m_WebhookTestBtn)
		{
			SendSimpleRpc(SM_PartyRPC.TEST_CLAN_WEBHOOK);
			return true;
		}

		if (w == m_WebhookAuditBtn)
		{
			SendSimpleRpc(SM_PartyRPC.TOGGLE_CLAN_AUDIT_WEBHOOK);
			return true;
		}

		if (w == m_SaveDescBtn)
		{
			string newDesc;
			m_DescEdit.GetText(newDesc);
			SendStringRpc(SM_PartyRPC.SET_DESCRIPTION, newDesc);
			m_DescMode = false;
			ShowTab();
			return true;
		}

		if (w == m_CloseDescBtn)
		{
			m_DescMode = false;
			ShowTab();
			return true;
		}

		if (w == m_ApplyBtn)
		{
			if (!SM_ClanClientData.HasClan && SM_ClanClientData.InfoName != "")
				SendStringRpc(SM_PartyRPC.APPLY_TO_CLAN, SM_ClanClientData.InfoName);
			m_InfoMode = false;
			ShowTab();
			return true;
		}

		if (w == m_AdminEnterClanBtn)
		{
			if (SM_ClanClientData.IsAdmin && SM_ClanClientData.InfoName != "")
				SendStringRpc(SM_PartyRPC.ADMIN_ENTER_CLAN, SM_ClanClientData.InfoName);
			m_InfoMode = false;
			ShowTab();
			return true;
		}

		if (w == m_CloseInfoBtn)
		{
			m_InfoMode = false;
			ShowTab();
			return true;
		}

		if (w == m_MapCloseBtn)
		{
			Close();
			return true;
		}

		if (w == m_MapModePrevBtn)
		{
			CycleMapMarkerMode(-1);
			return true;
		}

		if (w == m_MapModeNextBtn)
		{
			CycleMapMarkerMode(1);
			return true;
		}

		if (w == m_MapAddMarkerBtn)
		{
			NormalizeMapMarkerMode();
			if (!IsMapMarkerModeEditable())
				return true;

			if (!m_HasSelectedMapPosition)
			{
				if (m_MapSelectedText)
					m_MapSelectedText.SetText(SM_PartyLoc.Text("#STR_SMP_00946"));
				return true;
			}

			string mapMarkerName = m_MapMarkerNameEdit.GetText();
			mapMarkerName = mapMarkerName.Trim();
			if (mapMarkerName == "")
			{
				if (m_MapSelectedText)
					m_MapSelectedText.SetText(SM_PartyLoc.Text("#STR_SMP_00315"));
				return true;
			}

			if (m_MapMarkerMode == MAP_MARKER_MODE_PERSONAL)
			{
				string personalError;
				if (!SM_ClanClientData.AddPersonalMapMarker(mapMarkerName, m_SelectedMapPosition, m_SelectedMarkerColor, m_SelectedMarkerIcon, personalError))
				{
					if (m_MapSelectedText)
						m_MapSelectedText.SetText(SM_PartyLoc.Text(personalError));
					return true;
				}
			}
			else if (m_MapMarkerMode == MAP_MARKER_MODE_CLAN)
			{
				if (!SM_ClanClientData.HasClan)
					return true;

				Man markerPlayer = GetGame().GetPlayer();
				if (markerPlayer)
				{
					ScriptRPC markerRpc = new ScriptRPC();
					markerRpc.Write(m_SelectedMapPosition);
					markerRpc.Write(mapMarkerName);
					markerRpc.Write(m_SelectedMarkerColor);
					markerRpc.Write(m_SelectedMarkerIcon);
					markerRpc.Send(markerPlayer, SM_PartyRPC.MAP_MARKER_ADD, true, NULL);
				}
			}

			m_HasSelectedMapPosition = false;
			m_MapMarkerNameEdit.SetText(SM_PartyLoc.Text(""));
			RefreshMapMarks();
			RefreshMapMarkerList();
			return true;
		}

		if (w == m_MapRemoveMarkerBtn)
		{
			int markerRow = m_MapMarkersList.GetSelectedRow();
			if (markerRow >= 0 && markerRow < m_MapMarkerRowIds.Count() && markerRow < m_MapMarkerRowTypes.Count())
			{
				int markerType = m_MapMarkerRowTypes[markerRow];
				if (markerType == MAP_MARKER_ROW_PERSONAL)
				{
					SM_ClanClientData.RemovePersonalMapMarker(m_MapMarkerRowIds[markerRow]);
					HideMapMarkerEditor();
					RefreshMapMarks();
					RefreshMapMarkerList();
				}
				else if (markerType == MAP_MARKER_ROW_CLAN)
				{
					if (SM_ClanClientData.HasClan)
						SendIntRpc(SM_PartyRPC.MAP_MARKER_REMOVE, m_MapMarkerRowIds[markerRow]);
					HideMapMarkerEditor();
				}
			}
			return true;
		}

		if (w == m_MapColorBtn)
		{
			NormalizeMapMarkerMode();
			if (!IsMapMarkerModeEditable())
				return true;

			m_SelectedMarkerColor++;
			if (m_SelectedMarkerColor >= SM_ClanColors.Count())
				m_SelectedMarkerColor = 0;
			UpdateMapColorAndToggle();
			RefreshMapMarks();
			return true;
		}

		if (w == m_MapShowMeBtn)
		{
			ShowLocalPlayerOnMap();
			return true;
		}

		if (w == m_MapToggle3DBtn)
		{
			Toggle3DForSelectedMarker();
			return true;
		}

		if (w == m_MapAdminBasesBtn)
		{
			if (SM_ClanClientData.IsAdmin && SM_ClanClientData.AdminBaseMapMarkers.Count() > 0)
			{
				if (m_ShowAdminBaseMarkers)
					m_ShowAdminBaseMarkers = false;
				else
					m_ShowAdminBaseMarkers = true;

				HideMapMarkerEditor();
				RefreshMapMarks();
				RefreshMapMarkerList();
			}
			return true;
		}

		if (w == m_ClanMap)
			return true;

		if (w == m_TabMarketBtn)
		{
			m_CurrentTab = 7;
			ShowTab();
			SendSimpleRpc(SM_PartyRPC.MARKET_LIST);
			return true;
		}

		if (w == m_TabServerMarketBtn)
		{
			m_CurrentTab = 8;
			ShowTab();
			RequestServerMarketFullRefresh();
			return true;
		}

		if (w == m_TabStorageBtn)
		{
			m_CurrentTab = 9;
			ShowTab();
			SendSimpleRpc(SM_PartyRPC.REQUEST_STORAGE);
			return true;
		}

		if (w == m_TabAuctionBtn)
		{
			m_CurrentTab = 10;
			ShowTab();
			SendSimpleRpc(SM_PartyRPC.AUCTION_LIST);
			return true;
		}

		if (w == m_TabAchievementsBtn)
		{
			m_CurrentTab = 11;
			ShowTab();
			RequestAchievements();
			return true;
		}

		if (w == m_TabTitlesBtn)
		{
			m_CurrentTab = 13;
			ShowTab();
			SendSimpleRpc(SM_PartyRPC.REQUEST_PLAYER_TITLES);
			return true;
		}

		if (w == m_PlayerTitlesRefreshBtn)
		{
			SendSimpleRpc(SM_PartyRPC.REQUEST_PLAYER_TITLES);
			return true;
		}

		if (w == m_ContractRefreshBtn)
		{
			RequestContracts();
			return true;
		}

		if (w == m_ContractCreateKillBtn)
		{
			string targetUid = GetSelectedContractTargetUid();
			int price = GetContractPriceInput();
			int duration = GetContractDurationInput();
			if (targetUid != "" && price > 0)
				SendContractCreateKill(targetUid, price, duration);
			else if (targetUid == "" && m_ContractInfoText)
				m_ContractInfoText.SetText(SM_PartyLoc.Text("#STR_SMP_00364"));
			else if (price <= 0 && m_ContractInfoText)
				m_ContractInfoText.SetText(SM_PartyLoc.Text("#STR_SMP_00318"));
			else
				UpdateContractInfo();
			return true;
		}

		if (w == m_ContractCreateItemBtn)
		{
			string itemClass = GetSelectedContractItemClass();
			int itemPrice = GetContractPriceInput();
			int itemDuration = GetContractDurationInput();
			int itemQuantity = GetContractQuantityInput();
			if (itemClass != "" && itemPrice > 0)
				SendContractCreateItem(itemClass, itemQuantity, itemPrice, itemDuration);
			else if (itemClass == "" && m_ContractInfoText)
				m_ContractInfoText.SetText(SM_PartyLoc.Text("#STR_SMP_00360"));
			else if (itemPrice <= 0 && m_ContractInfoText)
				m_ContractInfoText.SetText(SM_PartyLoc.Text("#STR_SMP_00318"));
			else
				UpdateContractInfo();
			return true;
		}

		if (w == m_ContractAcceptBtn)
		{
			int selectedContractId = GetSelectedContractId();
			if (selectedContractId > 0)
				SendContractAction(SM_ContractRPCAction.ACCEPT, selectedContractId);
			else if (m_ContractInfoText)
				m_ContractInfoText.SetText(SM_PartyLoc.Text("#STR_SMP_00356"));
			return true;
		}

		if (w == m_ContractAbandonBtn)
		{
			int abandonContractId = GetSelectedContractId();
			if (abandonContractId > 0)
				SendContractAction(SM_ContractRPCAction.ABANDON, abandonContractId);
			else if (m_ContractInfoText)
				m_ContractInfoText.SetText(SM_PartyLoc.Text("#STR_SMP_00362"));
			return true;
		}

		if (w == m_ContractTurnInBtn)
		{
			int turnInContractId = GetSelectedContractId();
			if (turnInContractId > 0)
				SendContractAction(SM_ContractRPCAction.TURN_IN_ITEM, turnInContractId);
			else if (m_ContractInfoText)
				m_ContractInfoText.SetText(SM_PartyLoc.Text("#STR_SMP_00357"));
			return true;
		}

		if (w == m_AchPersonalBtn)
		{
			m_AchievementScopeFilter = SM_AchievementScope.PERSONAL;
			RefreshAchievements();
			return true;
		}

		if (w == m_AchClanBtn)
		{
			m_AchievementScopeFilter = SM_AchievementScope.CLAN;
			RefreshAchievements();
			return true;
		}

		if (w == m_ClaimAchievementBtn)
		{
			int achRow = m_AchievementsList.GetSelectedRow();
			if (achRow >= 0 && achRow < m_AchievementRowIds.Count() && achRow < m_AchievementRowScopes.Count())
				SendAchievementClaim(m_AchievementRowScopes[achRow], m_AchievementRowIds[achRow]);
			return true;
		}

		if (w == m_AuctionBidBtn)
		{
			int auctionBidRow = m_AuctionList.GetSelectedRow();
			SM_AuctionLotView auctionBidLot = GetAuctionLotByRow(auctionBidRow);
			if (auctionBidLot)
				SendAuctionBid(auctionBidLot.Id, auctionBidLot.GetNextBid());
			return true;
		}

		if (w == m_AuctionCancelBtn)
		{
			int auctionCancelRow = m_AuctionList.GetSelectedRow();
			if (auctionCancelRow >= 0 && auctionCancelRow < m_AuctionRowIds.Count())
				SendIntRpc(SM_PartyRPC.AUCTION_CANCEL, m_AuctionRowIds[auctionCancelRow]);
			return true;
		}

		if (w == m_AuctionSellBtn)
		{
			string startPriceText = "";
			string bidStepText = "";
			string durationText = "";
			if (m_AuctionStartEdit)
				startPriceText = m_AuctionStartEdit.GetText();
			if (m_AuctionStepEdit)
				bidStepText = m_AuctionStepEdit.GetText();
			if (m_AuctionDurationEdit)
				durationText = m_AuctionDurationEdit.GetText();

			int startPrice = startPriceText.ToInt();
			int bidStep = bidStepText.ToInt();
			int durationMinutes = durationText.ToInt();
			if (startPrice > 0)
				SendAuctionSell(startPrice, bidStep, durationMinutes);
			else if (m_AuctionInfoText)
				m_AuctionInfoText.SetText(SM_PartyLoc.Text("#STR_SMP_00316"));
			return true;
		}

		if (w == m_DepositToStorageBtn)
		{
			SendSimpleRpc(SM_PartyRPC.STORAGE_DEPOSIT);
			return true;
		}

		if (w == m_TakeFromStorageBtn)
		{
			int takeRow = m_StorageList.GetSelectedRow();
			if (takeRow >= 0 && takeRow < m_StorageRowIds.Count())
				SendIntRpc(SM_PartyRPC.STORAGE_TAKE, m_StorageRowIds[takeRow]);
			return true;
		}

		if (w == m_BuyLotBtn)
		{
			int buyRow = m_MarketList.GetSelectedRow();
			if (buyRow >= 0 && buyRow < m_MarketRowIds.Count())
				SendIntRpc(SM_PartyRPC.MARKET_BUY, m_MarketRowIds[buyRow]);
			return true;
		}

		if (w == m_BuyServerMarketBtn)
		{
			int serverBuyRow = m_ServerMarketList.GetSelectedRow();
			SM_ServerMarketItemView serverItem = GetServerMarketItemByRow(serverBuyRow);
			if (serverItem && !serverItem.Locked && !serverItem.IsSoldOut() && serverItem.GetLimitLeft() > 0)
			{
				m_ServerMarketFullRefreshRequested = true;
				SendIntRpc(SM_PartyRPC.SERVER_MARKET_BUY, serverItem.Id);
			}
			return true;
		}

		if (w == m_CancelLotBtn)
		{
			int cancelRow = m_MarketList.GetSelectedRow();
			if (cancelRow >= 0 && cancelRow < m_MarketRowIds.Count())
				SendIntRpc(SM_PartyRPC.MARKET_CANCEL, m_MarketRowIds[cancelRow]);
			return true;
		}

		if (w == m_SellLotBtn)
		{
			string priceText = m_SellPriceEdit.GetText();
			int sellPrice = priceText.ToInt();
			if (sellPrice > 0)
			{
				SendIntRpc(SM_PartyRPC.MARKET_SELL, sellPrice);
			}
			else
			{
				m_MarketInfoText.SetText(SM_PartyLoc.Text("#STR_SMP_00317"));
			}
			return true;
		}

		for (int c = 0; c < m_ColorBtns.Count(); c++)
		{
			if (w == m_ColorBtns[c])
			{
				SendIntRpc(SM_PartyRPC.SET_COLOR, c);
				return true;
			}
		}

		return super.OnClick(w, x, y, button);
	}

	protected void OnCreateClicked()
	{
		string name;
		string tag;
		string desc;
		GetCreateFormValues(name, tag, desc);

		name = name.Trim();
		tag = tag.Trim();
		desc = desc.Trim();

		int nameLength = name.LengthUtf8();
		if (nameLength < SM_ClanClientData.MinClanNameLength || nameLength > SM_ClanClientData.MaxClanNameLength)
		{
			m_CreateHintText.SetColor(ARGB(255, 235, 80, 80));
			m_CreateHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00669" + SM_ClanClientData.MinClanNameLength.ToString() + "#STR_SMP_00039" + SM_ClanClientData.MaxClanNameLength.ToString() + "#STR_SMP_00083"));
			return;
		}

		if (tag.LengthUtf8() > SM_ClanClientData.MaxClanTagLength)
		{
			m_CreateHintText.SetColor(ARGB(255, 235, 80, 80));
			m_CreateHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00986" + SM_ClanClientData.MaxClanTagLength.ToString() + "#STR_SMP_00083"));
			return;
		}

		if (tag != "" && !SM_PartyUtil.IsLatinAlnum(tag))
		{
			m_CreateHintText.SetColor(ARGB(255, 235, 80, 80));
			m_CreateHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00988"));
			return;
		}

		if (desc.LengthUtf8() > SM_ClanClientData.MaxClanDescriptionLength)
		{
			m_CreateHintText.SetColor(ARGB(255, 235, 80, 80));
			m_CreateHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00736" + SM_ClanClientData.MaxClanDescriptionLength.ToString() + "#STR_SMP_00083"));
			return;
		}

		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(name);
		rpc.Write(tag);
		rpc.Write(desc);
		rpc.Send(GetGame().GetPlayer(), SM_PartyRPC.CREATE_CLAN, true, NULL);
	}

	protected void ClearItemTileEntities(array<EntityAI> entities)
	{
		foreach (EntityAI entity : entities)
		{
			if (entity)
				GetGame().ObjectDelete(entity);
		}
		entities.Clear();
	}

	protected void ClearItemTiles(array<Widget> widgets, array<EntityAI> entities)
	{
		ClearItemTileEntities(entities);
		foreach (Widget tile : widgets)
		{
			if (tile)
				tile.Unlink();
		}
		widgets.Clear();
	}

	protected void ResizeTileRoot(Widget root, int tileCount)
	{
		if (!root)
			return;

		int columns = 1;
		int rowStep = 112;

		int rows = tileCount / columns;
		if ((tileCount % columns) > 0)
			rows++;
		if (rows < 6)
			rows = 6;

		float rootWidth;
		float rootHeight;
		root.GetSize(rootWidth, rootHeight);
		if (rootWidth <= 0)
			rootWidth = 1.0;

		root.SetSize(rootWidth, rows * rowStep);
	}

	protected Widget CreateItemTile(Widget root, array<Widget> widgets)
	{
		if (!root)
			return NULL;

		Widget tile = GetGame().GetWorkspace().CreateWidgets("SM_PartyMod/GUI/layouts/SM_ItemTile.layout", root);
		if (!tile)
			return NULL;

		int index = widgets.Count();
		int tileHeight = 100;
		int rowStep = 112;
		int row = index;

		tile.ClearFlags(WidgetFlags.HEXACTPOS);
		tile.ClearFlags(WidgetFlags.HEXACTSIZE);
		tile.SetSize(1.0, tileHeight);
		tile.SetPos(0.0, row * rowStep);
		widgets.Insert(tile);
		ResizeTileRoot(root, widgets.Count());
		return tile;
	}

	protected bool ShouldBuildItemTiles(Widget root)
	{
		if (!root)
			return false;
		if (root == m_MarketTilesRoot && m_CurrentTab != 7)
			return false;
		if (root == m_ServerMarketTilesRoot && m_CurrentTab != 8)
			return false;
		if (root == m_StorageTilesRoot && m_CurrentTab != 9)
			return false;
		if (root == m_AuctionTilesRoot && m_CurrentTab != 10)
			return false;
		if (!root.IsVisible())
			return false;
		return true;
	}

	protected void ApplyItemTileVisualState(Widget tile, bool disabled, bool selected)
	{
		if (!tile)
			return;

		Widget accent = tile.FindAnyWidget("TileAccent");
		Widget border = tile.FindAnyWidget("TileBorder");
		Widget surface = tile.FindAnyWidget("TileSurface");
		Widget previewBg = tile.FindAnyWidget("TilePreviewBg");
		Widget infoBand = tile.FindAnyWidget("TileInfoBand");
		Widget bottomLine = tile.FindAnyWidget("TileBottomLine");
		Widget disabledShade = tile.FindAnyWidget("TileDisabledShade");

		if (disabledShade)
			disabledShade.Show(disabled);

		if (selected)
		{
			tile.SetColor(ARGB(0, 0, 0, 0));
			if (accent)
				accent.SetColor(ARGB(255, 184, 115, 51));
			if (border)
				border.SetColor(ARGB(255, 33, 28, 23));
			if (surface)
				surface.SetColor(ARGB(245, 61, 52, 43));
			if (previewBg)
				previewBg.Show(false);
			if (infoBand)
				infoBand.SetColor(ARGB(246, 51, 44, 36));
			if (bottomLine)
				bottomLine.SetColor(ARGB(220, 44, 37, 31));
		}
		else if (disabled)
		{
			tile.SetColor(ARGB(0, 0, 0, 0));
			if (accent)
				accent.SetColor(ARGB(0, 169, 106, 47));
			if (border)
				border.SetColor(ARGB(150, 42, 36, 30));
			if (surface)
				surface.SetColor(ARGB(190, 37, 31, 26));
			if (previewBg)
				previewBg.Show(false);
			if (infoBand)
				infoBand.SetColor(ARGB(190, 34, 29, 24));
			if (bottomLine)
				bottomLine.SetColor(ARGB(120, 55, 47, 39));
		}
		else
		{
			tile.SetColor(ARGB(0, 0, 0, 0));
			if (accent)
				accent.SetColor(ARGB(0, 169, 106, 47));
			if (border)
				border.SetColor(ARGB(255, 33, 28, 23));
			if (surface)
				surface.SetColor(ARGB(245, 36, 31, 25));
			if (previewBg)
				previewBg.Show(false);
			if (infoBand)
				infoBand.SetColor(ARGB(245, 28, 24, 20));
			if (bottomLine)
				bottomLine.SetColor(ARGB(220, 41, 35, 29));
		}
	}

	protected void SetTileText(Widget tile, string name, string meta1, string meta2, string meta3, float progress, bool disabled, bool selected)
	{
		if (!tile)
			return;

		ApplyItemTileVisualState(tile, disabled, selected);

		TextWidget nameText = TextWidget.Cast(tile.FindAnyWidget("TileNameText"));
		TextWidget meta1Text = TextWidget.Cast(tile.FindAnyWidget("TileMeta1Text"));
		TextWidget meta2Text = TextWidget.Cast(tile.FindAnyWidget("TileMeta2Text"));
		TextWidget meta3Text = TextWidget.Cast(tile.FindAnyWidget("TileMeta3Text"));
		if (nameText)
		{
			nameText.SetText(SM_PartyLoc.Text(name));
			if (disabled)
				nameText.SetColor(ARGB(180, 171, 157, 137));
			else
				nameText.SetColor(ARGB(255, 250, 240, 228));
		}
		if (meta1Text)
		{
			meta1Text.SetText(SM_PartyLoc.Text(meta1));
			if (disabled)
				meta1Text.SetColor(ARGB(170, 171, 157, 137));
			else
				meta1Text.SetColor(ARGB(255, 217, 200, 174));
		}
		if (meta2Text)
		{
			meta2Text.SetText(SM_PartyLoc.Text(meta2));
			if (disabled)
				meta2Text.SetColor(ARGB(170, 164, 151, 131));
			else
				meta2Text.SetColor(ARGB(255, 226, 141, 63));
		}
		if (meta3Text)
		{
			meta3Text.SetText(SM_PartyLoc.Text(meta3));
			if (disabled)
				meta3Text.SetColor(ARGB(155, 147, 135, 117));
			else
				meta3Text.SetColor(ARGB(255, 177, 163, 142));
		}

		Widget track = tile.FindAnyWidget("TileProgressTrack");
		Widget fill = tile.FindAnyWidget("TileProgressFill");
		if (track)
		{
			track.Show(false);
		}
	}

	protected void SetTilePlaceholder(Widget tile, string text, bool visible)
	{
		if (!tile)
			return;

		TextWidget placeholder = TextWidget.Cast(tile.FindAnyWidget("TilePlaceholderText"));
		if (!placeholder)
			return;

		placeholder.Show(visible);
		if (visible)
			placeholder.SetText(SM_PartyLoc.Text(text));
	}

	protected void SelectItemRowOrFirst(TextListboxWidget list, int selectedRow)
	{
		if (!list)
			return;

		if (selectedRow >= 0)
		{
			list.SelectRow(selectedRow);
			return;
		}

		if (list.GetNumItems() > 0)
			list.SelectRow(0);
	}

	protected EntityAI BuildItemTilePreview(Widget tile, string className, array<string> attachClasses, array<EntityAI> entities)
	{
		if (!tile || className == "")
			return NULL;

		SetTilePlaceholder(tile, "", false);

		ItemPreviewWidget preview = ItemPreviewWidget.Cast(tile.FindAnyWidget("TilePreview"));
		if (!preview)
			return NULL;

		EntityAI entity = EntityAI.Cast(GetGame().CreateObjectEx(className, "0 0 0", ECE_LOCAL));
		if (!entity)
			return NULL;

		if (attachClasses)
		{
			foreach (string attachClass : attachClasses)
			{
				entity.GetInventory().CreateAttachment(attachClass);
			}
		}

		preview.SetItem(entity);
		preview.SetView(entity.GetViewIndex());
		preview.SetModelPosition(Vector(0, 0, 1));
		preview.SetModelOrientation("0 0 0");
		preview.Show(true);
		entities.Insert(entity);
		return entity;
	}

	protected void UpdateItemTileSelection(array<Widget> widgets, int selectedRow)
	{
		for (int i = 0; i < widgets.Count(); i++)
		{
			Widget tile = widgets[i];
			if (!tile)
				continue;

			bool selected = false;
			if (i == selectedRow)
				selected = true;

			bool disabled = false;
			Widget disabledShade = tile.FindAnyWidget("TileDisabledShade");
			if (disabledShade && disabledShade.IsVisible())
				disabled = true;

			ApplyItemTileVisualState(tile, disabled, selected);
		}
	}

	protected int FindItemTileIndex(Widget w, array<Widget> widgets)
	{
		for (int i = 0; i < widgets.Count(); i++)
		{
			if (w == widgets[i])
				return i;
		}
		return -1;
	}

	// Регистронезависимое совпадение подстроки. Пустой фильтр пропускает всё.
	protected bool MatchesSearch(string name, string filter)
	{
		if (filter == "")
			return true;

		string nameLower = name;
		nameLower.ToLower();
		string filterLower = filter;
		filterLower.ToLower();
		return nameLower.Contains(filterLower);
	}

	// Из-за фильтра строка списка больше не равна индексу в массиве данных,
	// поэтому ищем лот/предмет по сохранённому id строки.
	protected SM_MarketLotView GetMarketLotByRow(int row)
	{
		if (row < 0 || row >= m_MarketRowIds.Count())
			return NULL;

		int id = m_MarketRowIds[row];
		foreach (SM_MarketLotView lot : SM_ClanClientData.MarketLots)
		{
			if (lot.Id == id)
				return lot;
		}
		return NULL;
	}

	protected SM_ServerMarketItemView GetServerMarketItemByRow(int row)
	{
		if (row < 0 || row >= m_ServerMarketRowIds.Count())
			return NULL;

		int id = m_ServerMarketRowIds[row];
		foreach (SM_ServerMarketItemView item : SM_ClanClientData.ServerMarketItems)
		{
			if (item.Id == id)
				return item;
		}
		return NULL;
	}

	protected SM_StorageItemView GetStorageItemByRow(int row)
	{
		if (row < 0 || row >= m_StorageRowIds.Count())
			return NULL;

		int id = m_StorageRowIds[row];
		foreach (SM_StorageItemView item : SM_ClanClientData.StorageItems)
		{
			if (item.Id == id)
				return item;
		}
		return NULL;
	}

	protected SM_AuctionLotView GetAuctionLotByRow(int row)
	{
		if (row < 0 || row >= m_AuctionRowIds.Count())
			return NULL;

		int id = m_AuctionRowIds[row];
		foreach (SM_AuctionLotView lot : SM_ClanClientData.AuctionLots)
		{
			if (lot.Id == id)
				return lot;
		}
		return NULL;
	}

	protected SM_AchievementView GetAchievementByRow(int row)
	{
		if (row < 0 || row >= m_AchievementRowIds.Count() || row >= m_AchievementRowScopes.Count())
			return NULL;

		string id = m_AchievementRowIds[row];
		int scope = m_AchievementRowScopes[row];
		foreach (SM_AchievementView achievement : SM_ClanClientData.AchievementViews)
		{
			if (achievement.Id == id && achievement.Scope == scope)
				return achievement;
		}
		return NULL;
	}

	override bool OnChange(Widget w, int x, int y, bool finished)
	{
		if (w == m_MapMarkerEditRedSlider || w == m_MapMarkerEditGreenSlider || w == m_MapMarkerEditBlueSlider)
		{
			ApplyMapMarkerEditorRgbFromSliders(true);
			return true;
		}

		if (w == m_MarketSearchEdit)
		{
			m_MarketFilter = m_MarketSearchEdit.GetText();
			RefreshMarket();
			return true;
		}

		if (w == m_ServerMarketSearchEdit)
		{
			m_ServerMarketFilter = m_ServerMarketSearchEdit.GetText();
			RefreshServerMarket();
			return true;
		}

		if (w == m_AuctionSearchEdit)
		{
			m_AuctionFilter = m_AuctionSearchEdit.GetText();
			RefreshAuction();
			return true;
		}

		if (w == m_StorageSearchEdit)
		{
			m_StorageFilter = m_StorageSearchEdit.GetText();
			RefreshStorage();
			return true;
		}

		if (w == m_ContractsList || w == m_ContractTargetsList || w == m_ContractItemsList || w == m_ContractHistoryList)
		{
			UpdateContractInfo();
			return true;
		}

		return super.OnChange(w, x, y, finished);
	}

	void RefreshMarket()
	{
		if (!m_MarketList)
			return;

		int selectedId = -1;
		SM_MarketLotView selectedLot = GetMarketLotByRow(m_MarketList.GetSelectedRow());
		if (selectedLot)
			selectedId = selectedLot.Id;
		int selectedRow = -1;

		m_MarketList.ClearItems();
		m_MarketRowIds.Clear();
		ClearItemTiles(m_MarketTileWidgets, m_MarketTileEntities);
		bool buildMarketTiles = ShouldBuildItemTiles(m_MarketTilesRoot);

		int clanLots = 0;
		int shownLots = 0;
		foreach (SM_MarketLotView lot : SM_ClanClientData.MarketLots)
		{
			bool ownClanLot = SM_ClanClientData.HasClan && lot.ClanName == SM_ClanClientData.ClanName;
			if (ownClanLot)
				clanLots++;

			// Фильтр поиска по названию (счётчик лотов клана выше считаем до фильтра,
			// чтобы лимит казны отображался корректно).
			if (!MatchesSearch(lot.GetDisplayName(), m_MarketFilter))
				continue;

			string qty = "1";
			if (lot.Quantity >= 0)
			{
				int q = lot.Quantity;
				qty = q.ToString();
			}
			if (lot.AttachCount > 0)
				qty = qty + " (+" + lot.AttachCount.ToString() + ")";

			int row = m_MarketList.AddItem(SM_PartyLoc.Text(lot.GetDisplayName()), NULL, 0);
			m_MarketList.SetItem(row, SM_PartyLoc.Text(qty), NULL, 1);
			m_MarketList.SetItem(row, SM_PartyLoc.Text(lot.Price.ToString()), NULL, 2);
			m_MarketList.SetItem(row, SM_PartyLoc.Text(lot.ClanName), NULL, 3);
			m_MarketRowIds.Insert(lot.Id);
			if (lot.Id == selectedId)
				selectedRow = row;
			shownLots++;

			if (ownClanLot)
				m_MarketList.SetItemColor(row, 3, SM_ClanClientData.GetClanColor());

			if (buildMarketTiles)
			{
				Widget tile = CreateItemTile(m_MarketTilesRoot, m_MarketTileWidgets);
				if (tile)
				{
					string meta1 = qty;
					string meta2 = lot.Price.ToString();
					string meta3 = lot.ClanName;
					SetTileText(tile, lot.GetDisplayName(), meta1, meta2, meta3, lot.Health, false, lot.Id == selectedId);
					BuildItemTilePreview(tile, lot.ClassName, lot.AttachClasses, m_MarketTileEntities);
				}
			}
		}
		SelectItemRowOrFirst(m_MarketList, selectedRow);
		UpdateItemTileSelection(m_MarketTileWidgets, m_MarketList.GetSelectedRow());

		string marketHeader = "#STR_SMP_00892";
		if (SM_ClanClientData.MarketLots.Count() == 0)
			marketHeader = marketHeader + "#STR_SMP_00007";
		else if (m_MarketFilter != "" && shownLots == 0)
			marketHeader = marketHeader + "#STR_SMP_00008";

		// Слоты лотов своего клана (по аналогии со складом «занято/лимит»).
		// Лимит лотов есть только у участника клана.
		if (SM_ClanClientData.HasClan)
			marketHeader = marketHeader + " (" + clanLots.ToString() + "/" + SM_ClanClientData.LotCap.ToString() + ")";

		m_MarketHeaderText.SetText(SM_PartyLoc.Text(marketHeader));

		if (m_MarketSearchHint)
			m_MarketSearchHint.Show(m_MarketFilter == "");

		bool canSell = SM_ClanClientData.HasClan && SM_ClanClientData.MyRank >= SM_ClanClientData.GetActionRank(SM_ClanAction.MARKET_SELL);
		m_SellLabel.Show(canSell);
		m_SellPriceEdit.Show(canSell);
		m_SellPriceEditBg.Show(canSell);
		ShowEditUnderline("SellPriceEdit", canSell);
		m_SellLotBtn.Show(canSell);
		ShowBtnDressing("SellLotBtn", canSell);
		m_CancelLotBtn.Show(false);
		ShowBtnDressing("CancelLotBtn", false);

		string marketHint = "#STR_SMP_00405" + SM_ClanClientData.MarketFeePercent.ToString() + "%. #STR_SMP_00572" + SM_ClanClientData.MarketListingFeePercent.ToString() + "%.";
		if (SM_ClanClientData.HasClan)
		{
			marketHint = marketHint + "#STR_SMP_00056" + clanLots.ToString() + "/" + SM_ClanClientData.LotCap.ToString() + ".";
		}
		else
		{
			marketHint = marketHint + "#STR_SMP_00055" + SM_ClanClientData.LotCap.ToString() + "#STR_SMP_00022";
		}
		m_MarketHintText.SetText(SM_PartyLoc.Text(marketHint));

		UpdateMarketInfo();
	}

	protected void ClearMarketPreview()
	{
		if (m_MarketPreview)
		{
			m_MarketPreview.SetItem(NULL);
			m_MarketPreview.Show(false);
		}
		if (m_MarketPreviewEntity)
		{
			GetGame().ObjectDelete(m_MarketPreviewEntity);
			m_MarketPreviewEntity = NULL;
		}
	}

	protected void BuildMarketPreview(SM_MarketLotView lot)
	{
		ClearMarketPreview();

		if (!m_MarketPreview)
			return;

		m_MarketPreviewEntity = EntityAI.Cast(GetGame().CreateObjectEx(lot.ClassName, "0 0 0", ECE_LOCAL));
		if (!m_MarketPreviewEntity)
			return;

		foreach (string attachClass : lot.AttachClasses)
		{
			m_MarketPreviewEntity.GetInventory().CreateAttachment(attachClass);
		}

		m_MarketPreview.SetItem(m_MarketPreviewEntity);
		m_MarketPreview.SetModelOrientation("0 0 0");
		m_MarketPreview.Show(true);
	}

	protected void UpdateMarketInfo()
	{
		SM_MarketLotView lot = GetMarketLotByRow(m_MarketList.GetSelectedRow());
		if (!lot)
		{
			m_MarketInfoText.SetText(SM_PartyLoc.Text("#STR_SMP_00358"));
			if (m_BuyLotBtn)
				m_BuyLotBtn.Enable(false);
			SetBtnDressingState("BuyLotBtn", false);
			m_CancelLotBtn.Show(false);
			ShowBtnDressing("CancelLotBtn", false);
			ClearMarketPreview();
			return;
		}

		if (m_BuyLotBtn)
			m_BuyLotBtn.Enable(true);
		SetBtnDressingState("BuyLotBtn", true);

		if (lot.SellerUid == SM_ClanClientData.MyUid)
		{
			m_CancelLotBtn.Show(true);
			ShowBtnDressing("CancelLotBtn", true);
		}
		else
		{
			m_CancelLotBtn.Show(false);
			ShowBtnDressing("CancelLotBtn", false);
		}

		BuildMarketPreview(lot);
		int hp = Math.Round(lot.Health * 100);
		string info = lot.GetDisplayName();
		info = info + "\n" + "#STR_SMP_01112" + hp.ToString() + "%";
		if (lot.AttachCount > 0)
			info = info + "\n" + "#STR_SMP_01113" + lot.AttachCount.ToString();
		if (lot.Quantity >= 0)
		{
			int q = lot.Quantity;
			info = info + "\n" + "#STR_SMP_01114" + q.ToString();
		}
		info = info + "\n" + "#STR_SMP_01115" + lot.Price.ToString();
		info = info + "\n" + "#STR_SMP_01116" + lot.ClanName;
		info = info + "\n" + "#STR_SMP_01117" + lot.SellerName;
		info = info + "\n" + "#STR_SMP_01118" + lot.Date;
		m_MarketInfoText.SetText(SM_PartyLoc.Text(info));
	}

	protected string FormatServerMarketTimer(int seconds)
	{
		if (seconds < 0)
			seconds = 0;
		int hours = seconds / 3600;
		int minutes = (seconds % 3600) / 60;
		int secs = seconds % 60;
		return SM_PartyUtil.FormatTwoDigits(hours) + ":" + SM_PartyUtil.FormatTwoDigits(minutes) + ":" + SM_PartyUtil.FormatTwoDigits(secs);
	}

	protected string GetServerMarketPriceText(SM_ServerMarketItemView item)
	{
		if (!item || item.Locked || item.IsSoldOut())
			return "-";

		string text = item.CurrentPrice.ToString();
		if (item.BasePrice > 0 && item.CurrentPrice > item.BasePrice)
		{
			int percent = Math.Round((item.CurrentPrice - item.BasePrice) * 100.0 / item.BasePrice);
			text = text + " (+" + percent.ToString() + "%)";
		}
		return text;
	}

	protected string GetServerMarketLimitText(SM_ServerMarketItemView item)
	{
		if (!item)
			return "-";
		if (item.Locked)
		{
			if (item.RequiredClanLevel >= 0)
				return "#STR_SMP_01040" + item.RequiredClanLevel.ToString();
			return "#STR_SMP_00439";
		}
		if (item.MaxBuyPerPlayer <= 0)
			return "#STR_SMP_00272";

		int left = item.GetLimitLeft();
		return left.ToString() + "/" + item.MaxBuyPerPlayer.ToString();
	}

	protected string GetServerMarketQuantityText(SM_ServerMarketItemView item)
	{
		if (!item || item.Locked)
			return "-";
		if (item.IsSoldOut())
			return "#STR_SMP_00884";
		return item.Quantity.ToString() + "/" + item.InitialQuantity.ToString();
	}

	protected void ColorServerMarketRow(int row, int color)
	{
		for (int col = 0; col < 4; col++)
			m_ServerMarketList.SetItemColor(row, col, color);
	}

	protected bool ShouldShowServerMarketItem(SM_ServerMarketItemView item)
	{
		if (!item)
			return false;
		if (!item.Locked && !MatchesSearch(item.GetDisplayName(), m_ServerMarketFilter))
			return false;
		if (item.Locked && m_ServerMarketFilter != "")
			return false;
		return true;
	}

	protected int CountShownServerMarketItems()
	{
		int shown = 0;
		foreach (SM_ServerMarketItemView item : SM_ClanClientData.ServerMarketItems)
		{
			if (ShouldShowServerMarketItem(item))
				shown++;
		}
		return shown;
	}

	protected void UpdateServerMarketHeader(int shown)
	{
		string header = "#STR_SMP_00913";
		if (SM_ClanClientData.ServerMarketTotalItems > 0)
			header = header + " (" + SM_ClanClientData.ServerMarketVisibleItems.ToString() + "/" + SM_ClanClientData.ServerMarketTotalItems.ToString() + ")";
		if (SM_ClanClientData.ServerMarketItems.Count() == 0)
			header = header + "#STR_SMP_00013";
		else if (m_ServerMarketFilter != "" && shown == 0)
			header = header + "#STR_SMP_00008";
		header = header + "#STR_SMP_00010" + FormatServerMarketTimer(SM_ClanClientData.ServerMarketNextRefreshSeconds);
		m_ServerMarketHeaderText.SetText(SM_PartyLoc.Text(header));

		if (m_ServerMarketSearchHint)
			m_ServerMarketSearchHint.Show(m_ServerMarketFilter == "");
	}

	protected string BuildServerMarketInfoText(SM_ServerMarketItemView item)
	{
		string info = item.GetDisplayName();
		info = info + "\n" + "#STR_SMP_01119" + GetServerMarketQuantityText(item);
		info = info + "\n" + "#STR_SMP_01115" + GetServerMarketPriceText(item);
		if (item.MaxBuyPerPlayer > 0)
			info = info + "\n" + "#STR_SMP_01120" + item.BoughtByPlayer.ToString() + "/" + item.MaxBuyPerPlayer.ToString();
		else
			info = info + "\n" + "#STR_SMP_01129";
		info = info + "\n" + "#STR_SMP_01121" + FormatServerMarketTimer(SM_ClanClientData.ServerMarketNextRefreshSeconds);
		return info;
	}

	protected void UpdateServerMarketBuyButton(SM_ServerMarketItemView item)
	{
		if (!item)
		{
			m_BuyServerMarketBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00594"));
			m_BuyServerMarketBtn.Enable(false);
			SetBtnDressingState("BuyServerMarketBtn", false);
		}
		else if (item.IsSoldOut())
		{
			m_BuyServerMarketBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00884"));
			m_BuyServerMarketBtn.Enable(false);
			SetBtnDressingState("BuyServerMarketBtn", false);
		}
		else if (item.GetLimitLeft() <= 0)
		{
			m_BuyServerMarketBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00603"));
			m_BuyServerMarketBtn.Enable(false);
			SetBtnDressingState("BuyServerMarketBtn", false);
		}
		else
		{
			m_BuyServerMarketBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00594"));
			m_BuyServerMarketBtn.Enable(true);
			SetBtnDressingState("BuyServerMarketBtn", true);
		}
	}

	protected void UpdateServerMarketTimerView()
	{
		if (!m_ServerMarketList)
			return;

		UpdateServerMarketHeader(CountShownServerMarketItems());

		SM_ServerMarketItemView item = GetServerMarketItemByRow(m_ServerMarketList.GetSelectedRow());
		if (!item || item.Locked)
			return;

		m_ServerMarketInfoText.SetText(SM_PartyLoc.Text(BuildServerMarketInfoText(item)));
		UpdateServerMarketBuyButton(item);
	}

	protected void RequestServerMarketFullRefresh()
	{
		if (m_ServerMarketFullRefreshRequested)
			return;

		m_ServerMarketFullRefreshRequested = true;
		SendSimpleRpc(SM_PartyRPC.SERVER_MARKET_LIST);
	}

	protected void HandleServerMarketChanged()
	{
		bool rotationChanged = SM_ClanClientData.ServerMarketRotationId != m_ServerMarketShownRotationId;

		if (m_CurrentTab == 8)
		{
			if (m_ServerMarketFullRefreshRequested || rotationChanged)
			{
				RefreshServerMarket();
			}
			else
			{
				UpdateServerMarketTimerView();
			}
			return;
		}

		RefreshServerMarket();
	}

	void RefreshServerMarket()
	{
		if (!m_ServerMarketList)
			return;

		int selectedId = -1;
		SM_ServerMarketItemView selectedItem = GetServerMarketItemByRow(m_ServerMarketList.GetSelectedRow());
		if (selectedItem)
			selectedId = selectedItem.Id;
		int selectedRow = -1;

		m_ServerMarketList.ClearItems();
		m_ServerMarketRowIds.Clear();
		ClearItemTiles(m_ServerMarketTileWidgets, m_ServerMarketTileEntities);
		bool buildServerMarketTiles = ShouldBuildItemTiles(m_ServerMarketTilesRoot);

		int shown = 0;
		for (int i = 0; i < SM_ClanClientData.ServerMarketItems.Count(); i++)
		{
			SM_ServerMarketItemView item = SM_ClanClientData.ServerMarketItems[i];
			if (!item)
				continue;

			if (!ShouldShowServerMarketItem(item))
				continue;

			string name = item.GetDisplayName();
			if (item.Locked)
				name = "#STR_SMP_00933" + (i + 1).ToString();

			int row = m_ServerMarketList.AddItem(SM_PartyLoc.Text(name), NULL, 0);
			m_ServerMarketList.SetItem(row, SM_PartyLoc.Text(GetServerMarketQuantityText(item)), NULL, 1);
			m_ServerMarketList.SetItem(row, SM_PartyLoc.Text(GetServerMarketPriceText(item)), NULL, 2);
			m_ServerMarketList.SetItem(row, SM_PartyLoc.Text(GetServerMarketLimitText(item)), NULL, 3);
			m_ServerMarketRowIds.Insert(item.Id);
			if (item.Id == selectedId)
				selectedRow = row;
			shown++;

			if (item.Locked)
				ColorServerMarketRow(row, ARGB(255, 132, 121, 106));
			else if (item.IsSoldOut())
				ColorServerMarketRow(row, ARGB(255, 180, 95, 95));
			else if (item.GetLimitLeft() <= 0)
				ColorServerMarketRow(row, ARGB(255, 210, 160, 50));

			if (buildServerMarketTiles)
			{
				Widget tile = CreateItemTile(m_ServerMarketTilesRoot, m_ServerMarketTileWidgets);
				if (tile)
				{
					bool disabled = false;
					if (item.Locked || item.IsSoldOut() || item.GetLimitLeft() <= 0)
						disabled = true;

					float stockProgress = -1.0;
					if (!item.Locked && item.InitialQuantity > 0)
					{
						stockProgress = item.Quantity;
						stockProgress = stockProgress / item.InitialQuantity;
					}

					string meta1;
					string meta2;
					string meta3;
					if (item.Locked)
					{
						meta1 = "#STR_SMP_00439";
						meta2 = "";
						if (item.RequiredClanLevel >= 0)
							meta3 = "ур. " + item.RequiredClanLevel.ToString();
						else
							meta3 = "";
					}
					else
					{
						meta1 = GetServerMarketQuantityText(item);
						meta2 = GetServerMarketPriceText(item);
						meta3 = GetServerMarketLimitText(item);
						if (item.IsSoldOut())
							meta1 = "#STR_SMP_00884";
						else if (item.GetLimitLeft() <= 0)
							meta3 = "#STR_SMP_00603";
					}
					SetTileText(tile, name, meta1, meta2, meta3, stockProgress, disabled, item.Id == selectedId);
					if (!item.Locked)
					{
						array<string> emptyAttachments = new array<string>;
						BuildItemTilePreview(tile, item.ClassName, emptyAttachments, m_ServerMarketTileEntities);
					}
					else
					{
						SetTilePlaceholder(tile, "#STR_SMP_00439", true);
					}
				}
			}
		}

		UpdateServerMarketHeader(shown);
		m_ServerMarketHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00253"));
		SelectItemRowOrFirst(m_ServerMarketList, selectedRow);
		UpdateItemTileSelection(m_ServerMarketTileWidgets, m_ServerMarketList.GetSelectedRow());
		m_ServerMarketShownRotationId = SM_ClanClientData.ServerMarketRotationId;
		m_ServerMarketFullRefreshRequested = false;
		UpdateServerMarketInfo();
	}

	protected void ClearServerMarketPreview()
	{
		if (m_ServerMarketPreview)
		{
			m_ServerMarketPreview.SetItem(NULL);
			m_ServerMarketPreview.Show(false);
		}
		if (m_ServerMarketPreviewEntity)
		{
			GetGame().ObjectDelete(m_ServerMarketPreviewEntity);
			m_ServerMarketPreviewEntity = NULL;
		}
	}

	protected void BuildServerMarketPreview(SM_ServerMarketItemView item)
	{
		ClearServerMarketPreview();

		if (!m_ServerMarketPreview || !item || item.Locked)
			return;

		m_ServerMarketPreviewEntity = EntityAI.Cast(GetGame().CreateObjectEx(item.ClassName, "0 0 0", ECE_LOCAL));
		if (!m_ServerMarketPreviewEntity)
			return;

		m_ServerMarketPreview.SetItem(m_ServerMarketPreviewEntity);
		m_ServerMarketPreview.SetModelOrientation("0 0 0");
		m_ServerMarketPreview.Show(true);
	}

	protected void UpdateServerMarketInfo()
	{
		if (!m_BuyServerMarketBtn)
			return;

		SM_ServerMarketItemView item = GetServerMarketItemByRow(m_ServerMarketList.GetSelectedRow());
		if (!item)
		{
			m_ServerMarketInfoText.SetText(SM_PartyLoc.Text("#STR_SMP_00363"));
			m_BuyServerMarketBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00594"));
			m_BuyServerMarketBtn.Enable(false);
			SetBtnDressingState("BuyServerMarketBtn", false);
			ClearServerMarketPreview();
			return;
		}

		if (item.Locked)
		{
			if (item.RequiredClanLevel >= 0)
				m_ServerMarketInfoText.SetText(SM_PartyLoc.Text("#STR_SMP_00934" + item.RequiredClanLevel.ToString() + "."));
			else
				m_ServerMarketInfoText.SetText(SM_PartyLoc.Text("#STR_SMP_00935"));
			m_BuyServerMarketBtn.SetText(SM_PartyLoc.Text("#STR_SMP_00439"));
			m_BuyServerMarketBtn.Enable(false);
			SetBtnDressingState("BuyServerMarketBtn", false);
			ClearServerMarketPreview();
			return;
		}

		BuildServerMarketPreview(item);

		m_ServerMarketInfoText.SetText(SM_PartyLoc.Text(BuildServerMarketInfoText(item)));
		UpdateServerMarketBuyButton(item);
	}

	protected string FormatAuctionTimer(int seconds)
	{
		return FormatServerMarketTimer(seconds);
	}

	protected string GetAuctionBidText(SM_AuctionLotView lot)
	{
		if (!lot)
			return "-";
		if (lot.CurrentBid > 0)
			return lot.CurrentBid.ToString();
		return "-";
	}

	void RefreshAuction()
	{
		if (!m_AuctionList)
			return;

		int selectedId = -1;
		SM_AuctionLotView selectedLot = GetAuctionLotByRow(m_AuctionList.GetSelectedRow());
		if (selectedLot)
			selectedId = selectedLot.Id;
		int selectedRow = -1;

		m_AuctionList.ClearItems();
		m_AuctionRowIds.Clear();
		ClearItemTiles(m_AuctionTileWidgets, m_AuctionTileEntities);
		bool buildAuctionTiles = ShouldBuildItemTiles(m_AuctionTilesRoot);

		int shown = 0;
		foreach (SM_AuctionLotView lot : SM_ClanClientData.AuctionLots)
		{
			if (!lot)
				continue;
			if (!MatchesSearch(lot.GetDisplayName(), m_AuctionFilter))
				continue;

			int row = m_AuctionList.AddItem(SM_PartyLoc.Text(lot.GetDisplayName()), NULL, 0);
			m_AuctionList.SetItem(row, SM_PartyLoc.Text(lot.StartPrice.ToString()), NULL, 1);
			m_AuctionList.SetItem(row, SM_PartyLoc.Text(GetAuctionBidText(lot)), NULL, 2);
			m_AuctionList.SetItem(row, SM_PartyLoc.Text(FormatAuctionTimer(lot.SecondsLeft)), NULL, 3);
			m_AuctionList.SetItem(row, SM_PartyLoc.Text(lot.SellerName), NULL, 4);
			m_AuctionRowIds.Insert(lot.Id);
			if (lot.Id == selectedId)
				selectedRow = row;
			shown++;

			if (lot.SellerUid == SM_ClanClientData.MyUid)
				m_AuctionList.SetItemColor(row, 4, SM_ClanClientData.GetClanColor());
			if (lot.SecondsLeft <= 0)
			{
				for (int col = 0; col < 5; col++)
					m_AuctionList.SetItemColor(row, col, ARGB(255, 165, 152, 132));
			}

			if (buildAuctionTiles)
			{
				Widget tile = CreateItemTile(m_AuctionTilesRoot, m_AuctionTileWidgets);
				if (tile)
				{
					bool disabled = false;
					if (lot.SecondsLeft <= 0)
						disabled = true;

					string meta1 = lot.StartPrice.ToString();
					string meta2 = GetAuctionBidText(lot);
					string meta3 = FormatAuctionTimer(lot.SecondsLeft);
					SetTileText(tile, lot.GetDisplayName(), meta1, meta2, meta3, lot.Health, disabled, lot.Id == selectedId);
					BuildItemTilePreview(tile, lot.ClassName, lot.AttachClasses, m_AuctionTileEntities);
				}
			}
		}

		string header = "#STR_SMP_00258";
		if (SM_ClanClientData.AuctionLots.Count() == 0)
			header = header + "#STR_SMP_00007";
		else if (m_AuctionFilter != "" && shown == 0)
			header = header + "#STR_SMP_00008";
		m_AuctionHeaderText.SetText(SM_PartyLoc.Text(header));

		if (m_AuctionSearchHint)
			m_AuctionSearchHint.Show(m_AuctionFilter == "");

		SelectItemRowOrFirst(m_AuctionList, selectedRow);
		UpdateItemTileSelection(m_AuctionTileWidgets, m_AuctionList.GetSelectedRow());

		if (m_AuctionStepEdit && m_AuctionStepEdit.GetText() == "")
			m_AuctionStepEdit.SetText(SM_PartyLoc.Text(SM_ClanClientData.AuctionMinBidStep.ToString()));
		if (m_AuctionDurationEdit && m_AuctionDurationEdit.GetText() == "")
			m_AuctionDurationEdit.SetText(SM_PartyLoc.Text(SM_ClanClientData.AuctionDefaultDurationMinutes.ToString()));

		string hint = "#STR_SMP_00572" + SM_ClanClientData.AuctionListingFee.ToString() + " + " + SM_ClanClientData.AuctionListingFeePercent.ToString() + "%#STR_SMP_00166" + SM_ClanClientData.AuctionSaleTaxPercent.ToString() + "#STR_SMP_00139" + SM_ClanClientData.AuctionMinDurationMinutes.ToString() + "-" + SM_ClanClientData.AuctionMaxDurationMinutes.ToString() + "#STR_SMP_00059";
		m_AuctionHintText.SetText(SM_PartyLoc.Text(hint));
		UpdateAuctionInfo();
	}

	protected void ClearAuctionPreview()
	{
		if (m_AuctionPreview)
		{
			m_AuctionPreview.SetItem(NULL);
			m_AuctionPreview.Show(false);
		}
		if (m_AuctionPreviewEntity)
		{
			GetGame().ObjectDelete(m_AuctionPreviewEntity);
			m_AuctionPreviewEntity = NULL;
		}
	}

	protected void BuildAuctionPreview(SM_AuctionLotView lot)
	{
		ClearAuctionPreview();

		if (!m_AuctionPreview || !lot)
			return;

		m_AuctionPreviewEntity = EntityAI.Cast(GetGame().CreateObjectEx(lot.ClassName, "0 0 0", ECE_LOCAL));
		if (!m_AuctionPreviewEntity)
			return;

		foreach (string attachClass : lot.AttachClasses)
			m_AuctionPreviewEntity.GetInventory().CreateAttachment(attachClass);

		m_AuctionPreview.SetItem(m_AuctionPreviewEntity);
		m_AuctionPreview.SetModelOrientation("0 0 0");
		m_AuctionPreview.Show(true);
	}

	protected void UpdateAuctionInfo()
	{
		if (!m_AuctionInfoText || !m_AuctionBidBtn || !m_AuctionCancelBtn)
			return;

		SM_AuctionLotView lot = GetAuctionLotByRow(m_AuctionList.GetSelectedRow());
		if (!lot)
		{
			m_AuctionInfoText.SetText(SM_PartyLoc.Text("#STR_SMP_00358"));
			m_AuctionBidBtn.Enable(false);
			m_AuctionCancelBtn.Enable(false);
			ClearAuctionPreview();
			return;
		}

		BuildAuctionPreview(lot);

		int hp = Math.Round(lot.Health * 100);
		string info = lot.GetDisplayName();
		info = info + "\n" + "#STR_SMP_01112" + hp.ToString() + "%";
		if (lot.AttachCount > 0)
			info = info + "\n" + "#STR_SMP_01113" + lot.AttachCount.ToString();
		if (lot.Quantity >= 0)
			info = info + "\n" + "#STR_SMP_01114" + lot.Quantity.ToString();
		info = info + "\n" + "#STR_SMP_01122" + lot.StartPrice.ToString();
		info = info + "\n" + "#STR_SMP_01123" + lot.BidStep.ToString();
		info = info + "\n" + "#STR_SMP_01124" + GetAuctionBidText(lot);
		if (lot.CurrentBidderName != "")
			info = info + "\n" + "#STR_SMP_01125" + lot.CurrentBidderName;
		info = info + "\n" + "#STR_SMP_01126" + lot.GetNextBid().ToString();
		info = info + "\n" + "#STR_SMP_01127" + FormatAuctionTimer(lot.SecondsLeft);
		info = info + "\n" + "#STR_SMP_01117" + lot.SellerName;
		if (lot.SellerClanName != "")
			info = info + " [" + lot.SellerClanName + "]";
		info = info + "\n" + "#STR_SMP_01118" + lot.Date;
		m_AuctionInfoText.SetText(SM_PartyLoc.Text(info));

		bool canBid = true;
		if (lot.SellerUid == SM_ClanClientData.MyUid)
			canBid = false;
		if (lot.SecondsLeft <= 0)
			canBid = false;
		m_AuctionBidBtn.Enable(canBid);

		bool canCancel = false;
		if (lot.SellerUid == SM_ClanClientData.MyUid && lot.CurrentBid <= 0)
			canCancel = true;
		m_AuctionCancelBtn.Enable(canCancel);
	}

	protected bool ShouldShowAchievement(SM_AchievementView achievement)
	{
		if (!achievement)
			return false;
		if (achievement.Scope != m_AchievementScopeFilter)
			return false;
		return true;
	}

	protected string FormatAchievementProgress(SM_AchievementView achievement)
	{
		if (!achievement)
			return "";

		string type = achievement.Type;
		if (type == "OnlineSeconds" || type == "BestLifeSeconds")
			return SM_PartyUtil.FormatDuration(achievement.Progress) + "/" + SM_PartyUtil.FormatDuration(achievement.Target);
		if (type == "DistanceWalked" || type == "LongestKill")
			return SM_PartyUtil.FormatDistance(achievement.Progress) + "/" + SM_PartyUtil.FormatDistance(achievement.Target);
		return achievement.Progress.ToString() + "/" + achievement.Target.ToString();
	}

	protected string FormatAchievementReward(SM_AchievementView achievement)
	{
		if (!achievement)
			return "";

		string reward = "";
		if (achievement.RewardMoney > 0)
			reward = reward + achievement.RewardMoney.ToString();
		if (achievement.RewardClanTreasury > 0)
		{
			if (reward != "")
				reward = reward + " + ";
			reward = reward + "#STR_SMP_00483" + achievement.RewardClanTreasury.ToString();
		}
		if (achievement.RewardContainerClass != "")
		{
			if (reward != "")
				reward = reward + " + ";
			reward = reward + SM_PartyUtil.GetItemDisplayName(achievement.RewardContainerClass);
		}
		if (reward == "")
			reward = "-";
		return reward;
	}

	protected void UpdateAchievementScopeButtons()
	{
		bool personalActive = (m_AchievementScopeFilter == SM_AchievementScope.PERSONAL);

		if (m_AchPersonalBg)
		{
			int pbg = ARGB(217, 45, 38, 32);
			if (personalActive)
				pbg = ARGB(242, 69, 59, 49);
			m_AchPersonalBg.SetColor(pbg);
		}
		if (m_AchClanBg)
		{
			int cbg = ARGB(217, 45, 38, 32);
			if (!personalActive)
				cbg = ARGB(242, 69, 59, 49);
			m_AchClanBg.SetColor(cbg);
		}
		if (m_AchPersonalBtn)
		{
			int ptx = ARGB(210, 196, 180, 157);
			if (personalActive)
				ptx = ARGB(255, 250, 240, 228);
			m_AchPersonalBtn.SetTextColor(ptx);
		}
		if (m_AchClanBtn)
		{
			int ctx = ARGB(210, 196, 180, 157);
			if (!personalActive)
				ctx = ARGB(255, 250, 240, 228);
			m_AchClanBtn.SetTextColor(ctx);
		}
		if (m_AchPersonalUnder)
			m_AchPersonalUnder.Show(personalActive);
		if (m_AchClanUnder)
			m_AchClanUnder.Show(!personalActive);
	}

	void RefreshAchievements()
	{
		if (!m_AchievementsList)
			return;

		if (m_AchievementScopeFilter == SM_AchievementScope.CLAN && !SM_ClanClientData.HasClan)
			m_AchievementScopeFilter = SM_AchievementScope.PERSONAL;

		UpdateAchievementScopeButtons();

		string selectedId = "";
		int selectedScope = -1;
		SM_AchievementView selectedAchievement = GetAchievementByRow(m_AchievementsList.GetSelectedRow());
		if (selectedAchievement)
		{
			selectedId = selectedAchievement.Id;
			selectedScope = selectedAchievement.Scope;
		}
		int selectedRow = -1;

		m_AchievementsList.ClearItems();
		m_AchievementRowIds.Clear();
		m_AchievementRowScopes.Clear();

		int shown = 0;
		foreach (SM_AchievementView achievement : SM_ClanClientData.AchievementViews)
		{
			if (!ShouldShowAchievement(achievement))
				continue;

			string status = FormatAchievementReward(achievement);
			if (achievement.Claimed)
				status = "#STR_SMP_00807";
			else if (achievement.Completed)
				status = "#STR_SMP_00385";

			int row = m_AchievementsList.AddItem(SM_PartyLoc.Text(achievement.Name), NULL, 0);
			m_AchievementsList.SetItem(row, SM_PartyLoc.Text(FormatAchievementProgress(achievement)), NULL, 1);
			m_AchievementsList.SetItem(row, SM_PartyLoc.Text(status), NULL, 2);
			m_AchievementRowIds.Insert(achievement.Id);
			m_AchievementRowScopes.Insert(achievement.Scope);

			if (achievement.Id == selectedId && achievement.Scope == selectedScope)
				selectedRow = row;

			if (achievement.Claimed)
			{
				for (int col = 0; col < 3; col++)
					m_AchievementsList.SetItemColor(row, col, ARGB(255, 143, 131, 114));
			}
			else if (achievement.Completed)
			{
				for (int readyCol = 0; readyCol < 3; readyCol++)
					m_AchievementsList.SetItemColor(row, readyCol, ARGB(255, 207, 129, 57));
			}
			shown++;
		}

		string header = "#STR_SMP_00612";
		if (m_AchievementScopeFilter == SM_AchievementScope.CLAN)
			header = "#STR_SMP_00564";
		if (shown == 0)
			header = header + "#STR_SMP_00011";
		m_AchievementsHeaderText.SetText(SM_PartyLoc.Text(header));

		if (m_AchPersonalBtn)
			m_AchPersonalBtn.Enable(m_AchievementScopeFilter != SM_AchievementScope.PERSONAL);
		if (m_AchClanBtn)
		{
			m_AchClanBtn.Show(SM_ClanClientData.HasClan);
			m_AchClanBtn.Enable(m_AchievementScopeFilter != SM_AchievementScope.CLAN);
		}

		if (selectedRow >= 0)
			m_AchievementsList.SelectRow(selectedRow);

		if (m_AchievementsHintText)
			m_AchievementsHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00613"));
		UpdateAchievementInfo();
	}

	protected void UpdateAchievementInfo()
	{
		if (!m_AchievementInfoText || !m_ClaimAchievementBtn)
			return;

		SM_AchievementView achievement = GetAchievementByRow(m_AchievementsList.GetSelectedRow());
		if (!achievement)
		{
			m_AchievementInfoText.SetText(SM_PartyLoc.Text("#STR_SMP_00354"));
			if (m_AchievementProgressTrack)
				m_AchievementProgressTrack.Show(false);
			m_ClaimAchievementBtn.Enable(false);
			return;
		}

		string info = achievement.Name;
		if (achievement.Description != "")
			info = info + "\n" + achievement.Description;
		info = info + "#STR_SMP_00113" + FormatAchievementProgress(achievement);
		info = info + "#STR_SMP_00109" + FormatAchievementReward(achievement);
		if (achievement.Claimed)
			info = info + "#STR_SMP_00124";
		else if (achievement.Completed)
			info = info + "#STR_SMP_00123";
		else
			info = info + "#STR_SMP_00122";
		m_AchievementInfoText.SetText(SM_PartyLoc.Text(info));

		if (m_AchievementProgressTrack && m_AchievementProgressFill)
		{
			float achievementProgress = 0.0;
			if (achievement.Target > 0)
			{
				achievementProgress = achievement.Progress;
				achievementProgress = achievementProgress / achievement.Target;
			}
			if (achievementProgress > 1.0)
				achievementProgress = 1.0;
			if (achievementProgress < 0.0)
				achievementProgress = 0.0;
			m_AchievementProgressFill.SetSize(achievementProgress, 1.0);
			m_AchievementProgressTrack.Show(true);
		}

		bool canClaim = false;
		if (achievement.Completed && !achievement.Claimed)
			canClaim = true;
		m_ClaimAchievementBtn.Enable(canClaim);
	}

	void RefreshPlayerTitles()
	{
		if (!m_PlayerTitlesList)
			return;

		m_PlayerTitlesList.ClearItems();

		if (!SM_ClanClientData.PlayerExperienceEnabled)
		{
			if (m_PlayerTitlesHeaderText)
				m_PlayerTitlesHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_00459"));
			if (m_PlayerTitleMineText)
				m_PlayerTitleMineText.SetText(SM_PartyLoc.Text(""));
			if (m_PlayerTitleProgressText)
				m_PlayerTitleProgressText.SetText(SM_PartyLoc.Text(""));
			if (m_PlayerTitleProgressTrack)
				m_PlayerTitleProgressTrack.Show(false);
			return;
		}

		if (m_PlayerTitlesHeaderText)
			m_PlayerTitlesHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_00460"));

		string mine = "#STR_SMP_00312" + SM_ClanClientData.MyPlayerTitle + " · " + SM_ClanClientData.MyPlayerXP.ToString() + " XP";
		if (m_PlayerTitleMineText)
		{
			m_PlayerTitleMineText.SetText(SM_PartyLoc.Text(mine));
			if (SM_ClanClientData.MyPlayerTitleColor != 0)
				m_PlayerTitleMineText.SetColor(SM_ClanClientData.MyPlayerTitleColor);
			else
				m_PlayerTitleMineText.SetColor(ARGB(255, 250, 240, 228));
		}

		string progressText = "#STR_SMP_00852" + SM_ClanClientData.MyPlayerXP.ToString() + " XP";
		float progress = 0.0;
		if (SM_ClanClientData.MyPlayerNextXP > 0)
		{
			progressText = "#STR_SMP_00414" + SM_ClanClientData.MyPlayerXP.ToString() + " / " + SM_ClanClientData.MyPlayerNextXP.ToString() + " XP";
			progress = SM_ClanClientData.MyPlayerXP;
			progress = progress / SM_ClanClientData.MyPlayerNextXP;
		}
		else if (SM_ClanClientData.MyPlayerXP > 0)
		{
			progressText = "#STR_SMP_00627";
			progress = 1.0;
		}
		if (progress < 0.0)
			progress = 0.0;
		if (progress > 1.0)
			progress = 1.0;
		if (m_PlayerTitleProgressText)
			m_PlayerTitleProgressText.SetText(SM_PartyLoc.Text(progressText));
		if (m_PlayerTitleProgressTrack && m_PlayerTitleProgressFill)
		{
			m_PlayerTitleProgressTrack.Show(true);
			m_PlayerTitleProgressFill.SetSize(progress, 1.0);
			if (SM_ClanClientData.MyPlayerTitleColor != 0)
				m_PlayerTitleProgressFill.SetColor(SM_ClanClientData.MyPlayerTitleColor);
			else
				m_PlayerTitleProgressFill.SetColor(ARGB(255, 184, 115, 51));
		}

		for (int i = 0; i < SM_ClanClientData.PlayerTitles.Count(); i++)
		{
			SM_PlayerTitleView view = SM_ClanClientData.PlayerTitles[i];
			if (!view)
				continue;

			string place = (i + 1).ToString();
			string name = view.Name;
			if (view.Uid == SM_ClanClientData.MyUid)
				name = name + "#STR_SMP_00016";

			string clanName = view.ClanName;
			if (clanName == "")
				clanName = "-";
			string status = "Offline";
			if (view.Online)
				status = "Online";

			int row = m_PlayerTitlesList.AddItem(SM_PartyLoc.Text(place), NULL, 0);
			m_PlayerTitlesList.SetItem(row, SM_PartyLoc.Text(name), NULL, 1);
			m_PlayerTitlesList.SetItem(row, SM_PartyLoc.Text(view.Title), NULL, 2);
			m_PlayerTitlesList.SetItem(row, SM_PartyLoc.Text(status), NULL, 3);
			m_PlayerTitlesList.SetItem(row, SM_PartyLoc.Text(clanName), NULL, 4);

			if (!view.Online)
			{
				for (int col = 0; col < 5; col++)
					m_PlayerTitlesList.SetItemColor(row, col, ARGB(255, 153, 141, 123));
			}
			else
			{
				if (view.Color != 0)
					m_PlayerTitlesList.SetItemColor(row, 2, view.Color);
				if (view.Uid == SM_ClanClientData.MyUid)
				{
					for (int myCol = 0; myCol < 5; myCol++)
						m_PlayerTitlesList.SetItemColor(row, myCol, SM_ClanClientData.GetClanColor());
					if (view.Color != 0)
						m_PlayerTitlesList.SetItemColor(row, 2, view.Color);
				}
			}
		}

		if (m_PlayerTitlesHintText)
			m_PlayerTitlesHintText.SetText(SM_PartyLoc.Text("#STR_SMP_00473"));
	}

	protected string FormatContractSubject(SM_ContractView view)
	{
		if (!view)
			return "";
		if (view.Type == SM_ContractType.KILL)
		{
			string target = view.TargetName;
			if (view.TargetClanName != "")
				target = target + " [" + view.TargetClanName + "]";
			return target;
		}
		return view.GetSubject();
	}

	protected int GetContractTypeColor(int contractType)
	{
		if (contractType == SM_ContractType.KILL)
			return ARGB(255, 235, 105, 95);
		if (contractType == SM_ContractType.ITEM)
			return ARGB(255, 228, 179, 71);
		return ARGB(255, 250, 240, 228);
	}

	protected int GetContractMapMarkerColor(int contractType)
	{
		if (contractType == SM_ContractType.KILL)
			return ARGB(255, 255, 40, 40);
		if (contractType == SM_ContractType.ITEM)
			return ARGB(255, 228, 179, 71);
		return ARGB(255, 207, 129, 57);
	}

	protected string GetContractMapMarkerIcon(int contractType)
	{
		if (contractType == SM_ContractType.KILL)
			return "SM_PartyMod\\GUI\\pings\\skull_ping.paa";
		return SM_ClanMarkerIcon.GetPath();
	}

	protected void ClearContractTargetPreview()
	{
		if (m_ContractTargetPreview)
		{
			m_ContractTargetPreview.UpdateItemInHands(NULL);
			m_ContractTargetPreview.SetPlayer(NULL);
			m_ContractTargetPreview.Show(false);
		}
		if (m_ContractTargetPreviewPanel)
			m_ContractTargetPreviewPanel.Show(false);
		if (m_ContractTargetPreviewHand)
		{
			GetGame().ObjectDelete(m_ContractTargetPreviewHand);
			m_ContractTargetPreviewHand = NULL;
		}
		if (m_ContractTargetPreviewPlayer)
		{
			Object previewObject = m_ContractTargetPreviewPlayer;
			if (previewObject)
				GetGame().ObjectDelete(previewObject);
			m_ContractTargetPreviewPlayer = NULL;
		}
	}

	void OnContractTargetPreviewChanged()
	{
		BuildContractTargetPreview();
	}

	protected void BuildContractTargetPreview()
	{
		ClearContractTargetPreview();

		if (!m_ContractTargetPreview || !m_ContractTargetPreviewPanel)
			return;

		string playerClass = SM_ClanClientData.ContractTargetPreviewClass;
		playerClass = playerClass.Trim();
		if (playerClass == "")
			playerClass = "SurvivorM_Mirek";

		Entity playerEntity = GetGame().CreatePlayer(NULL, playerClass, "0 0 0", 0, "NONE");
		m_ContractTargetPreviewPlayer = DayZPlayer.Cast(playerEntity);
		if (!m_ContractTargetPreviewPlayer)
		{
			if (m_ContractInfoText)
				m_ContractInfoText.SetText(SM_PartyLoc.Text("#STR_SMP_00826" + playerClass));
			return;
		}
		m_ContractTargetPreviewPlayer.SetPosition("0 0 0");
		m_ContractTargetPreviewPlayer.SetOrientation("0 0 0");
		m_ContractTargetPreviewPlayer.DisableSimulation(true);

		PlayerBase previewPlayer = PlayerBase.Cast(m_ContractTargetPreviewPlayer);
		if (previewPlayer)
		{
			previewPlayer.SetAllowDamage(false);
			previewPlayer.SetCanBeDestroyed(false);
			previewPlayer.SetModifiers(false);
		}

		EntityAI previewEntity = m_ContractTargetPreviewPlayer;
		if (previewEntity)
		{
			foreach (string attachmentClass : SM_ClanClientData.ContractTargetPreviewAttachments)
			{
				if (attachmentClass != "")
					previewEntity.GetInventory().CreateAttachment(attachmentClass);
			}
		}

		string handClass = SM_ClanClientData.ContractTargetPreviewHandClass;
		handClass = handClass.Trim();
		if (handClass != "")
		{
			m_ContractTargetPreviewHand = EntityAI.Cast(GetGame().CreateObjectEx(handClass, "0 0 0", ECE_LOCAL));
			if (m_ContractTargetPreviewHand)
				m_ContractTargetPreviewHand.DisableSimulation(true);
		}

		if (m_ContractTargetPreviewName)
			m_ContractTargetPreviewName.SetText(SM_PartyLoc.Text("#STR_SMP_01056" + SM_ClanClientData.ContractTargetPreviewName));

		m_ContractTargetPreview.SetPlayer(m_ContractTargetPreviewPlayer);
		if (m_ContractTargetPreviewHand)
			m_ContractTargetPreview.UpdateItemInHands(m_ContractTargetPreviewHand);
		else
			m_ContractTargetPreview.UpdateItemInHands(NULL);
		m_ContractTargetPreview.SetModelPosition("0 0 0.60");
		m_ContractTargetPreview.SetModelOrientation("0 0 0");
		m_ContractTargetPreview.Refresh();
		m_ContractTargetPreview.Show(true);
		m_ContractTargetPreviewPanel.Show(true);
	}

	void OnContractsChanged()
	{
		if (m_CurrentTab == 14 && !SM_ClanClientData.ContractsEnabled)
		{
			m_CurrentTab = 0;
			ShowTab();
		}
		RefreshTabs();
		RefreshContracts();
		if (m_CurrentTab == 6)
			RefreshMapMarks();
	}

	protected SM_ContractView GetContractViewById(int id)
	{
		foreach (SM_ContractView view : SM_ClanClientData.ContractViews)
		{
			if (view && view.Id == id)
				return view;
		}
		foreach (SM_ContractView historyView : SM_ClanClientData.ContractHistoryViews)
		{
			if (historyView && historyView.Id == id)
				return historyView;
		}
		return null;
	}

	protected SM_ContractView GetContractViewByRow(int row)
	{
		if (row < 0 || row >= m_ContractRowIds.Count())
			return null;
		return GetContractViewById(m_ContractRowIds[row]);
	}

	protected void TickContractTimers()
	{
		bool changed = false;
		bool expired = false;

		for (int i = 0; i < SM_ClanClientData.ContractViews.Count(); i++)
		{
			SM_ContractView view = SM_ClanClientData.ContractViews[i];
			if (!view)
				continue;
			if (view.SecondsLeft > 0)
			{
				view.SecondsLeft--;
				changed = true;
				if (view.SecondsLeft == 0)
					expired = true;
			}
		}

		for (int m = 0; m < SM_ClanClientData.ContractMarkers.Count(); m++)
		{
			SM_ContractMarkerView marker = SM_ClanClientData.ContractMarkers[m];
			if (!marker)
				continue;
			if (marker.SecondsLeft > 0)
				marker.SecondsLeft--;
		}

		if (changed)
			RefreshContractTimerTexts();
		if (expired)
			RequestContracts();
	}

	protected void RefreshContractTimerTexts()
	{
		if (m_ContractsList)
		{
			for (int row = 0; row < m_ContractRowIds.Count(); row++)
			{
				SM_ContractView view = GetContractViewById(m_ContractRowIds[row]);
				if (view)
					m_ContractsList.SetItem(row, SM_PartyLoc.Text(FormatAuctionTimer(view.SecondsLeft)), NULL, 3);
			}
		}

		UpdateContractInfo();
	}

	protected int GetSelectedContractId()
	{
		if (!m_ContractsList)
			return 0;
		int row = m_ContractsList.GetSelectedRow();
		if (row < 0 || row >= m_ContractRowIds.Count())
			return 0;
		return m_ContractRowIds[row];
	}

	protected string GetSelectedContractTargetUid()
	{
		if (!m_ContractTargetsList)
			return "";
		int row = m_ContractTargetsList.GetSelectedRow();
		if (row < 0 || row >= m_ContractTargetRowUids.Count())
			return "";
		return m_ContractTargetRowUids[row];
	}

	protected string GetSelectedContractItemClass()
	{
		if (!m_ContractItemsList)
			return "";
		int row = m_ContractItemsList.GetSelectedRow();
		if (row < 0 || row >= m_ContractItemRowClasses.Count())
			return "";
		return m_ContractItemRowClasses[row];
	}

	protected int GetContractPriceInput()
	{
		if (!m_ContractPriceEdit)
			return 0;
		string text = m_ContractPriceEdit.GetText();
		text = text.Trim();
		return text.ToInt();
	}

	protected int GetContractDurationInput()
	{
		if (!m_ContractDurationEdit)
			return SM_ClanClientData.ContractsMinDurationMinutes;
		string text = m_ContractDurationEdit.GetText();
		text = text.Trim();
		int duration = text.ToInt();
		if (duration <= 0)
			duration = SM_ClanClientData.ContractsMinDurationMinutes;
		return duration;
	}

	protected int GetContractQuantityInput()
	{
		if (!m_ContractQuantityEdit)
			return 1;
		string text = m_ContractQuantityEdit.GetText();
		text = text.Trim();
		int quantity = text.ToInt();
		if (quantity < 1)
			quantity = 1;
		return quantity;
	}

	protected void UpdateContractInfo()
	{
		if (!m_ContractInfoText)
			return;

		if (!SM_ClanClientData.ContractsEnabled)
		{
			m_ContractInfoText.SetText(SM_PartyLoc.Text("#STR_SMP_00918"));
			return;
		}

		SM_ContractView view = GetContractViewByRow(m_ContractsList.GetSelectedRow());
		if (!view)
		{
			string info = "#STR_SMP_01059" + SM_ClanClientData.ContractsMinPrice.ToString() + "-" + SM_ClanClientData.ContractsMaxPrice.ToString();
			info = info + "#STR_SMP_00105" + SM_ClanClientData.ContractsFeePercent.ToString() + "%";
			info = info + "#STR_SMP_00128" + SM_ClanClientData.ContractsPenaltyPercent.ToString() + "%";
			info = info + "#STR_SMP_00100" + SM_ClanClientData.ContractsMinDurationMinutes.ToString() + "-" + SM_ClanClientData.ContractsMaxDurationMinutes.ToString() + "#STR_SMP_00059";
			if (!SM_ClanClientData.ContractsCanCreate)
				info = info + "#STR_SMP_00118";
			m_ContractInfoText.SetText(SM_PartyLoc.Text(info));
			if (m_ContractAcceptBtn)
				m_ContractAcceptBtn.Enable(false);
			if (m_ContractAbandonBtn)
				m_ContractAbandonBtn.Enable(false);
			if (m_ContractTurnInBtn)
				m_ContractTurnInBtn.Enable(false);
			return;
		}

		string text = SM_ContractType.GetName(view.Type) + ": " + FormatContractSubject(view);
		text = text + "#STR_SMP_00121" + SM_ContractStatus.GetName(view.Status);
		text = text + "#STR_SMP_00117" + view.CreatorName;
		if (view.CreatorClanName != "")
			text = text + " [" + view.CreatorClanName + "]";
		if (view.ExecutorName != "")
		{
			text = text + "#STR_SMP_00102" + view.ExecutorName;
			if (view.ExecutorClanName != "")
				text = text + " [" + view.ExecutorClanName + "]";
		}
		text = text + "#STR_SMP_00126" + view.Price.ToString();
		text = text + "#STR_SMP_00109" + view.Reward.ToString();
		text = text + "#STR_SMP_00106" + view.Fee.ToString();
		text = text + "#STR_SMP_00128" + view.Penalty.ToString();
		text = text + "#STR_SMP_00111" + FormatAuctionTimer(view.SecondsLeft);
		text = text + "#STR_SMP_00099" + view.Date;
		m_ContractInfoText.SetText(SM_PartyLoc.Text(text));

		bool canAccept = false;
		if (view.Status == SM_ContractStatus.OPEN && !view.IsCreator && !view.IsExecutor)
			canAccept = true;
		bool canAbandon = false;
		if (view.Status == SM_ContractStatus.ACCEPTED && view.IsExecutor)
			canAbandon = true;
		bool canTurnIn = false;
		if (view.Status == SM_ContractStatus.ACCEPTED && view.Type == SM_ContractType.ITEM && view.IsExecutor)
			canTurnIn = true;

		if (m_ContractAcceptBtn)
			m_ContractAcceptBtn.Enable(canAccept);
		if (m_ContractAbandonBtn)
			m_ContractAbandonBtn.Enable(canAbandon);
		if (m_ContractTurnInBtn)
			m_ContractTurnInBtn.Enable(canTurnIn);
	}

	void RefreshContracts()
	{
		if (!m_ContractsList)
			return;

		if (m_ContractsHeaderText)
			m_ContractsHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_00583"));
		if (m_HdrContractType)
			m_HdrContractType.SetText(SM_PartyLoc.Text("#STR_SMP_00994"));
		if (m_HdrContractTarget)
			m_HdrContractTarget.SetText(SM_PartyLoc.Text("#STR_SMP_01054"));
		if (m_HdrContractReward)
			m_HdrContractReward.SetText(SM_PartyLoc.Text("#STR_SMP_00656"));
		if (m_HdrContractTime)
			m_HdrContractTime.SetText(SM_PartyLoc.Text("#STR_SMP_00339"));
		if (m_HdrContractStatus)
			m_HdrContractStatus.SetText(SM_PartyLoc.Text("#STR_SMP_00977"));
		if (m_ContractHistoryHeaderText)
			m_ContractHistoryHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_00480"));
		if (m_ContractTargetsHeaderText)
			m_ContractTargetsHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_01052"));
		if (m_ContractItemsHeaderText)
			m_ContractItemsHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_00830"));

		int selectedId = GetSelectedContractId();
		string selectedTargetUid = GetSelectedContractTargetUid();
		string selectedItemClass = GetSelectedContractItemClass();
		int selectedRow = -1;
		m_ContractsList.ClearItems();
		m_ContractRowIds.Clear();
		for (int i = 0; i < SM_ClanClientData.ContractViews.Count(); i++)
		{
			SM_ContractView view = SM_ClanClientData.ContractViews[i];
			if (!view)
				continue;

			int row = m_ContractsList.AddItem(SM_PartyLoc.Text(SM_ContractType.GetName(view.Type)), NULL, 0);
			m_ContractsList.SetItem(row, SM_PartyLoc.Text(FormatContractSubject(view)), NULL, 1);
			m_ContractsList.SetItem(row, SM_PartyLoc.Text(view.Reward.ToString()), NULL, 2);
			m_ContractsList.SetItem(row, SM_PartyLoc.Text(FormatAuctionTimer(view.SecondsLeft)), NULL, 3);
			m_ContractsList.SetItem(row, SM_PartyLoc.Text(SM_ContractStatus.GetName(view.Status)), NULL, 4);
			m_ContractRowIds.Insert(view.Id);
			if (view.Id == selectedId)
				selectedRow = row;

			int contractTypeColor = GetContractTypeColor(view.Type);
			if (view.IsExecutor)
			{
				for (int myCol = 0; myCol < 5; myCol++)
					m_ContractsList.SetItemColor(row, myCol, SM_ClanClientData.GetClanColor());
			}
			else if (view.IsCreator)
			{
				for (int creatorCol = 0; creatorCol < 5; creatorCol++)
					m_ContractsList.SetItemColor(row, creatorCol, ARGB(255, 210, 145, 83));
			}
			else if (view.Status == SM_ContractStatus.ACCEPTED)
			{
				for (int accCol = 0; accCol < 5; accCol++)
					m_ContractsList.SetItemColor(row, accCol, ARGB(255, 217, 199, 173));
			}
			m_ContractsList.SetItemColor(row, 0, contractTypeColor);
			m_ContractsList.SetItemColor(row, 1, contractTypeColor);
		}
		if (selectedRow >= 0)
			m_ContractsList.SelectRow(selectedRow);
		else if (m_ContractRowIds.Count() > 0)
			m_ContractsList.SelectRow(0);

		if (m_ContractHistoryList)
		{
			m_ContractHistoryList.ClearItems();
			m_ContractHistoryRowIds.Clear();
			for (int h = 0; h < SM_ClanClientData.ContractHistoryViews.Count(); h++)
			{
				SM_ContractView history = SM_ClanClientData.ContractHistoryViews[h];
				if (!history)
					continue;
				int hRow = m_ContractHistoryList.AddItem(SM_PartyLoc.Text(SM_ContractStatus.GetName(history.Status)), NULL, 0);
				m_ContractHistoryList.SetItem(hRow, SM_PartyLoc.Text(SM_ContractType.GetName(history.Type)), NULL, 1);
				m_ContractHistoryList.SetItem(hRow, SM_PartyLoc.Text(FormatContractSubject(history)), NULL, 2);
				m_ContractHistoryList.SetItem(hRow, SM_PartyLoc.Text(history.ExecutorName), NULL, 3);
				m_ContractHistoryList.SetItem(hRow, SM_PartyLoc.Text(history.Reward.ToString()), NULL, 4);
				int historyTypeColor = GetContractTypeColor(history.Type);
				m_ContractHistoryList.SetItemColor(hRow, 1, historyTypeColor);
				m_ContractHistoryList.SetItemColor(hRow, 2, historyTypeColor);
				m_ContractHistoryRowIds.Insert(history.Id);
			}
		}

		if (m_ContractTargetsList)
		{
			int selectedTargetRow = -1;
			m_ContractTargetsList.ClearItems();
			m_ContractTargetRowUids.Clear();
			for (int t = 0; t < SM_ClanClientData.ContractTargets.Count(); t++)
			{
				SM_ContractTargetView target = SM_ClanClientData.ContractTargets[t];
				if (!target)
					continue;
				string targetClan = target.ClanName;
				if (targetClan == "")
					targetClan = "-";
				int tRow = m_ContractTargetsList.AddItem(SM_PartyLoc.Text(target.Name), NULL, 0);
				m_ContractTargetsList.SetItem(tRow, SM_PartyLoc.Text(target.Title), NULL, 1);
				m_ContractTargetsList.SetItem(tRow, SM_PartyLoc.Text(targetClan), NULL, 2);
				m_ContractTargetRowUids.Insert(target.Uid);
				if (target.Uid == selectedTargetUid)
					selectedTargetRow = tRow;
			}
			if (selectedTargetRow >= 0)
				m_ContractTargetsList.SelectRow(selectedTargetRow);
			else if (m_ContractTargetRowUids.Count() > 0)
				m_ContractTargetsList.SelectRow(0);
		}

		if (m_ContractItemsList)
		{
			int selectedItemRow = -1;
			m_ContractItemsList.ClearItems();
			m_ContractItemRowClasses.Clear();
			for (int it = 0; it < SM_ClanClientData.ContractItemOptions.Count(); it++)
			{
				SM_ContractItemOptionView option = SM_ClanClientData.ContractItemOptions[it];
				if (!option)
					continue;
				int itemRow = m_ContractItemsList.AddItem(SM_PartyLoc.Text(option.DisplayName), NULL, 0);
				m_ContractItemsList.SetItem(itemRow, SM_PartyLoc.Text(option.ClassName), NULL, 1);
				m_ContractItemRowClasses.Insert(option.ClassName);
				if (option.ClassName == selectedItemClass)
					selectedItemRow = itemRow;
			}
			if (selectedItemRow >= 0)
				m_ContractItemsList.SelectRow(selectedItemRow);
			else if (m_ContractItemRowClasses.Count() > 0)
				m_ContractItemsList.SelectRow(0);
		}

		if (m_ContractCreateKillBtn)
			m_ContractCreateKillBtn.Enable(SM_ClanClientData.ContractsEnabled && SM_ClanClientData.ContractsCanCreate);
		if (m_ContractCreateItemBtn)
			m_ContractCreateItemBtn.Enable(SM_ClanClientData.ContractsEnabled && SM_ClanClientData.ContractsCanCreate);
		if (m_ContractPriceEdit)
		{
			int currentPrice = GetContractPriceInput();
			if (currentPrice < SM_ClanClientData.ContractsMinPrice || currentPrice > SM_ClanClientData.ContractsMaxPrice)
				m_ContractPriceEdit.SetText(SM_PartyLoc.Text(SM_ClanClientData.ContractsMinPrice.ToString()));
		}
		if (m_ContractDurationEdit)
		{
			int currentDuration = GetContractDurationInput();
			if (currentDuration < SM_ClanClientData.ContractsMinDurationMinutes || currentDuration > SM_ClanClientData.ContractsMaxDurationMinutes)
				m_ContractDurationEdit.SetText(SM_PartyLoc.Text(SM_ClanClientData.ContractsMinDurationMinutes.ToString()));
		}
		if (m_ContractQuantityEdit && m_ContractQuantityEdit.GetText() == "")
			m_ContractQuantityEdit.SetText(SM_PartyLoc.Text("1"));

		UpdateContractInfo();
	}

	void RefreshStorage()
	{
		if (!m_StorageList)
			return;

		int selectedId = -1;
		SM_StorageItemView selectedItem = GetStorageItemByRow(m_StorageList.GetSelectedRow());
		if (selectedItem)
			selectedId = selectedItem.Id;
		int selectedRow = -1;

		m_StorageList.ClearItems();
		m_StorageRowIds.Clear();
		ClearItemTiles(m_StorageTileWidgets, m_StorageTileEntities);
		bool buildStorageTiles = ShouldBuildItemTiles(m_StorageTilesRoot);

		int shownItems = 0;
		foreach (SM_StorageItemView item : SM_ClanClientData.StorageItems)
		{
			if (!MatchesSearch(item.GetDisplayName(), m_StorageFilter))
				continue;

			string qty = "1";
			if (item.Quantity >= 0)
			{
				int q = item.Quantity;
				qty = q.ToString();
			}
			if (item.AttachCount > 0)
				qty = qty + " (+" + item.AttachCount.ToString() + ")";

			int row = m_StorageList.AddItem(SM_PartyLoc.Text(item.GetDisplayName()), NULL, 0);
			m_StorageList.SetItem(row, SM_PartyLoc.Text(qty), NULL, 1);
			m_StorageList.SetItem(row, SM_PartyLoc.Text(item.DepositorName), NULL, 2);
			m_StorageList.SetItem(row, SM_PartyLoc.Text(item.Date), NULL, 3);
			m_StorageRowIds.Insert(item.Id);
			if (item.Id == selectedId)
				selectedRow = row;
			shownItems++;

			if (buildStorageTiles)
			{
				Widget tile = CreateItemTile(m_StorageTilesRoot, m_StorageTileWidgets);
				if (tile)
				{
					string meta1 = qty;
					string meta2 = item.DepositorName;
					string meta3 = item.Date;
					SetTileText(tile, item.GetDisplayName(), meta1, meta2, meta3, item.Health, false, item.Id == selectedId);
					BuildItemTilePreview(tile, item.ClassName, item.AttachClasses, m_StorageTileEntities);
				}
			}
		}
		SelectItemRowOrFirst(m_StorageList, selectedRow);
		UpdateItemTileSelection(m_StorageTileWidgets, m_StorageList.GetSelectedRow());

		string usage = SM_ClanClientData.StorageUsed.ToString() + "/" + SM_ClanClientData.StorageCap.ToString();
		if (SM_ClanClientData.StorageItems.Count() == 0)
			m_StorageHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_00926" + usage + ")"));
		else if (m_StorageFilter != "" && shownItems == 0)
			m_StorageHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_00924" + usage + ")"));
		else
			m_StorageHeaderText.SetText(SM_PartyLoc.Text("#STR_SMP_00925" + usage + ")"));

		if (m_StorageSearchHint)
			m_StorageSearchHint.Show(m_StorageFilter == "");

		m_DepositToStorageBtn.Show(SM_ClanClientData.HasClan && !SM_ClanClientData.IsAdminMode);
		ShowBtnDressing("DepositToStorageBtn", SM_ClanClientData.HasClan && !SM_ClanClientData.IsAdminMode);
		m_TakeFromStorageBtn.Show(SM_ClanClientData.HasClan && !SM_ClanClientData.IsAdminMode);
		ShowBtnDressing("TakeFromStorageBtn", SM_ClanClientData.HasClan && !SM_ClanClientData.IsAdminMode);

		bool atBase = false;
		if (SM_ClanClientData.HasBase)
		{
			Man me = GetGame().GetPlayer();
			if (me)
				atBase = vector.Distance(me.GetPosition(), SM_ClanClientData.BasePos) <= SM_ClanClientData.StorageRadius;
		}
		m_DepositToStorageBtn.Enable(atBase);
		m_TakeFromStorageBtn.Enable(atBase);

		string storageHint;
		if (!SM_ClanClientData.HasBase)
			storageHint = "#STR_SMP_00920";
		else if (!atBase)
			storageHint = "#STR_SMP_00959";
		else
			storageHint = "#STR_SMP_00337";
		m_StorageHintText.SetText(SM_PartyLoc.Text(storageHint));

		UpdateStorageInfo();
	}

	protected void ClearStoragePreview()
	{
		if (m_StoragePreview)
		{
			m_StoragePreview.SetItem(NULL);
			m_StoragePreview.Show(false);
		}
		if (m_StoragePreviewEntity)
		{
			GetGame().ObjectDelete(m_StoragePreviewEntity);
			m_StoragePreviewEntity = NULL;
		}
	}

	protected void BuildStoragePreview(SM_StorageItemView item)
	{
		ClearStoragePreview();

		if (!m_StoragePreview)
			return;

		m_StoragePreviewEntity = EntityAI.Cast(GetGame().CreateObjectEx(item.ClassName, "0 0 0", ECE_LOCAL));
		if (!m_StoragePreviewEntity)
			return;

		foreach (string attachClass : item.AttachClasses)
		{
			m_StoragePreviewEntity.GetInventory().CreateAttachment(attachClass);
		}

		m_StoragePreview.SetItem(m_StoragePreviewEntity);
		m_StoragePreview.SetModelOrientation("0 0 0");
		m_StoragePreview.Show(true);
	}

	protected void UpdateStorageInfo()
	{
		SM_StorageItemView item = GetStorageItemByRow(m_StorageList.GetSelectedRow());
		if (!item)
		{
			m_StorageInfoText.SetText(SM_PartyLoc.Text("#STR_SMP_00361"));
			ClearStoragePreview();
			return;
		}

		BuildStoragePreview(item);

		int hp = Math.Round(item.Health * 100);
		string info = item.GetDisplayName();
		info = info + "\n" + "#STR_SMP_01112" + hp.ToString() + "%";
		if (item.AttachCount > 0)
			info = info + "\n" + "#STR_SMP_01113" + item.AttachCount.ToString();
		if (item.Quantity >= 0)
		{
			int q = item.Quantity;
			info = info + "\n" + "#STR_SMP_01114" + q.ToString();
		}
		info = info + "\n" + "#STR_SMP_01128" + item.DepositorName;
		info = info + "\n" + "#STR_SMP_01118" + item.Date;
		m_StorageInfoText.SetText(SM_PartyLoc.Text(info));
	}

	override bool OnItemSelected(Widget w, int x, int y, int row, int column, int oldRow, int oldColumn)
	{
		if (w == m_MarketList)
		{
			UpdateMarketInfo();
			return true;
		}

		if (w == m_ServerMarketList)
		{
			UpdateServerMarketInfo();
			return true;
		}

		if (w == m_AuctionList)
		{
			UpdateAuctionInfo();
			return true;
		}

		if (w == m_AchievementsList)
		{
			UpdateAchievementInfo();
			return true;
		}

		if (w == m_StorageList)
		{
			UpdateStorageInfo();
			return true;
		}

		if (w == m_MapMarkersList)
		{
			Update3DToggleButton();
			RefreshMapControls();
			if (row >= 0 && row < m_MapMarkerRowIds.Count() && row < m_MapMarkerRowTypes.Count())
			{
				int selectedMarkerId = m_MapMarkerRowIds[row];
				int selectedMarkerType = m_MapMarkerRowTypes[row];

				if (selectedMarkerType == MAP_MARKER_ROW_PERSONAL)
				{
					foreach (SM_PersonalMapMarker personalMarker : SM_ClanClientData.PersonalMapMarkers)
					{
						if (personalMarker && personalMarker.Id == selectedMarkerId)
						{
							if (m_ClanMap)
								m_ClanMap.SetMapPos(personalMarker.Position);
							if (m_MapSelectedText)
								m_MapSelectedText.SetText(SM_PartyLoc.Text("#STR_SMP_00368" + personalMarker.Name));
							return true;
						}
					}
				}
				else if (selectedMarkerType == MAP_MARKER_ROW_CLAN)
				{
					foreach (SM_ClanMapMarker marker : SM_ClanClientData.MapMarkers)
					{
						if (marker.Id == selectedMarkerId)
						{
							if (m_ClanMap)
								m_ClanMap.SetMapPos(marker.Position);
							if (m_MapSelectedText)
								m_MapSelectedText.SetText(SM_PartyLoc.Text("#STR_SMP_00367" + marker.Name));
							return true;
						}
					}
				}
				else if (selectedMarkerType == MAP_MARKER_ROW_SERVER)
				{
					if (selectedMarkerId >= 0 && selectedMarkerId < SM_ClanClientData.ServerMapMarkers.Count())
					{
						SM_ServerMapMarker serverMarker = SM_ClanClientData.ServerMapMarkers[selectedMarkerId];
						if (serverMarker)
						{
							if (m_ClanMap)
								m_ClanMap.SetMapPos(serverMarker.Position);
							if (m_MapSelectedText)
								m_MapSelectedText.SetText(SM_PartyLoc.Text("#STR_SMP_00369" + serverMarker.Name));
							return true;
						}
					}
				}
				else if (selectedMarkerType == MAP_MARKER_ROW_ADMIN_BASE)
				{
					if (selectedMarkerId >= 0 && selectedMarkerId < SM_ClanClientData.AdminBaseMapMarkers.Count())
					{
						SM_AdminBaseMapMarker adminBaseMarker = SM_ClanClientData.AdminBaseMapMarkers[selectedMarkerId];
						if (adminBaseMarker)
						{
							if (m_ClanMap)
								m_ClanMap.SetMapPos(adminBaseMarker.Position);
							if (m_MapSelectedText)
								m_MapSelectedText.SetText(SM_PartyLoc.Text("#STR_SMP_00366" + adminBaseMarker.ClanName));
							return true;
						}
					}
				}
			}
			return true;
		}

		return super.OnItemSelected(w, x, y, row, column, oldRow, oldColumn);
	}

	protected string GetSelectedMemberUid()
	{
		int row = m_MembersList.GetSelectedRow();
		if (row >= 0 && row < m_MemberRowUids.Count())
			return m_MemberRowUids[row];
		return "";
	}

	protected void SendSimpleRpc(int rpcType)
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Send(player, rpcType, true, NULL);
	}

	protected void SendStringRpc(int rpcType, string value)
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(value);
		rpc.Send(player, rpcType, true, NULL);
	}

	protected void SendIntRpc(int rpcType, int value)
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(value);
		rpc.Send(player, rpcType, true, NULL);
	}

	protected void SendAuctionSell(int startPrice, int bidStep, int durationMinutes)
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(startPrice);
		rpc.Write(bidStep);
		rpc.Write(durationMinutes);
		rpc.Send(player, SM_PartyRPC.AUCTION_SELL, true, NULL);
	}

	protected void SendAuctionBid(int lotId, int bidAmount)
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(lotId);
		rpc.Write(bidAmount);
		rpc.Send(player, SM_PartyRPC.AUCTION_BID, true, NULL);
	}

	protected void RequestAchievements()
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(SM_AchievementRPCAction.REQUEST);
		rpc.Send(player, SM_PartyRPC.ACHIEVEMENTS, true, NULL);
	}

	protected void SendAchievementClaim(int scope, string achievementId)
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(SM_AchievementRPCAction.CLAIM);
		rpc.Write(scope);
		rpc.Write(achievementId);
		rpc.Send(player, SM_PartyRPC.ACHIEVEMENTS, true, NULL);
	}

	protected void RequestContracts()
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(SM_ContractRPCAction.REQUEST);
		rpc.Send(player, SM_PartyRPC.CONTRACTS, true, NULL);
	}

	protected void SendContractAction(int action, int contractId)
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(action);
		rpc.Write(contractId);
		rpc.Send(player, SM_PartyRPC.CONTRACTS, true, NULL);
	}

	protected void SendContractCreateKill(string targetUid, int price, int durationMinutes)
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(SM_ContractRPCAction.CREATE_KILL);
		rpc.Write(targetUid);
		rpc.Write(price);
		rpc.Write(durationMinutes);
		rpc.Send(player, SM_PartyRPC.CONTRACTS, true, NULL);
	}

	protected void SendContractCreateItem(string className, int quantity, int price, int durationMinutes)
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(SM_ContractRPCAction.CREATE_ITEM);
		rpc.Write(className);
		rpc.Write(quantity);
		rpc.Write(price);
		rpc.Write(durationMinutes);
		rpc.Send(player, SM_PartyRPC.CONTRACTS, true, NULL);
	}

	protected void SendContractTargetPreviewRequest(string targetUid)
	{
		Man player = GetGame().GetPlayer();
		if (!player)
			return;
		ScriptRPC rpc = new ScriptRPC();
		rpc.Write(SM_ContractRPCAction.REQUEST_TARGET_PREVIEW);
		rpc.Write(targetUid);
		rpc.Send(player, SM_PartyRPC.CONTRACTS, true, NULL);
	}
}
