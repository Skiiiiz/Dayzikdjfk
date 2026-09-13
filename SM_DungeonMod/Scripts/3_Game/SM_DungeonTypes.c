// ============================================================================
// SM_DungeonTypes.c
//
// Небольшая внутренняя "часовая" утилита + все данные, которые живут либо
// в JSON-базе мода ($profile:SM_DungeonMod\*.json - брони, приглашения,
// штрафы игроков), либо только в оперативной памяти сервера, пока идёт
// конкретный забег (SM_DungeonRunState - активные забеги переживают только
// работу сервера, при рестарте не восстанавливаются).
// ============================================================================

// Простые целочисленные "минуты от эпохи" на основе реального системного
// времени сервера (System.GetYearMonthDay/GetHourMinuteSecond). Этого
// достаточно для еженедельного календаря броней, тайм-аутов приглашений,
// временных блокировок за нарушения и КД на данж - собственный Unix-время
// в Enforce Script недоступен, но абсолютная монотонная шкала в минутах
// с точностью до реального времени сервера нам и нужна.
class SM_DungeonClock
{
	// Howard Hinnant's "days_from_civil" - конвертация календарной даты
	// в число дней от фиксированной точки отсчёта. Работает корректно для
	// любых реальных календарных дат.
	static int DaysFromCivil(int y, int m, int d)
	{
		if (m <= 2)
			y = y - 1;

		int era;
		if (y >= 0)
			era = y / 400;
		else
			era = (y - 399) / 400;

		int yoe = y - era * 400;
		int mAdj;
		if (m > 2)
			mAdj = m - 3;
		else
			mAdj = m + 9;
		int doy = (153 * mAdj + 2) / 5 + d - 1;
		int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
		return era * 146097 + doe - 719468;
	}

	static int NowMinutes()
	{
		int year, month, day, hour, minute, second;
		GetYearMonthDay(year, month, day);
		GetHourMinuteSecond(hour, minute, second);
		int days = DaysFromCivil(year, month, day);
		return days * 1440 + hour * 60 + minute;
	}

	// 0 = понедельник ... 6 = воскресенье. 1970-01-01 (день 0 от нашей
	// эпохи с точки зрения DaysFromCivil при отсчёте от 0000-03-01) был
	// четвергом, поэтому смещение +3 приводит понедельник к остатку 0.
	static int DayOfWeek(int minutesSinceEpoch)
	{
		int days = minutesSinceEpoch / 1440;
		int dow = (days + 3) % 7;
		if (dow < 0)
			dow += 7;
		return dow;
	}

	static int MinuteOfDay(int minutesSinceEpoch)
	{
		int m = minutesSinceEpoch % 1440;
		if (m < 0)
			m += 1440;
		return m;
	}

	// Обратное преобразование к DaysFromCivil (Howard Hinnant's
	// "civil_from_days") - нужно только для показа времени слота в UI.
	static void CivilFromDays(int z, out int year, out int month, out int day)
	{
		z += 719468;

		int era;
		if (z >= 0)
			era = z / 146097;
		else
			era = (z - 146096) / 146097;

		int doe = z - era * 146097;
		int yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
		int y = yoe + era * 400;
		int doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
		int mp = (5 * doy + 2) / 153;
		int d = doy - (153 * mp + 2) / 5 + 1;

		int m;
		if (mp < 10)
			m = mp + 3;
		else
			m = mp - 9;

		if (m <= 2)
			y = y + 1;

		year = y;
		month = m;
		day = d;
	}

	static string PadTwo(int value)
	{
		if (value < 10)
			return "0" + value.ToString();
		return value.ToString();
	}

	// "ДД.ММ ЧЧ:ММ" для абсолютной минуты, полученной от NowMinutes().
	static string FormatAbsoluteMinute(int minutesSinceEpoch)
	{
		int days = minutesSinceEpoch / 1440;
		int year, month, day;
		CivilFromDays(days, year, month, day);

		int minuteOfDay = MinuteOfDay(minutesSinceEpoch);
		int hour = minuteOfDay / 60;
		int minute = minuteOfDay % 60;

		return PadTwo(day) + "." + PadTwo(month) + " " + PadTwo(hour) + ":" + PadTwo(minute);
	}

	// "HH:MM" -> минута суток. Некорректный формат -> -1.
	static int ParseHHMM(string text)
	{
		text = text.Trim();
		if (text.Length() != 5 || text.Substring(2, 1) != ":")
			return -1;

		int hour = text.Substring(0, 2).ToInt();
		int minute = text.Substring(3, 2).ToInt();
		if (hour < 0 || hour > 23 || minute < 0 || minute > 59)
			return -1;

		return hour * 60 + minute;
	}
}

class SM_DungeonInvite
{
	string Id;
	string BookingId;
	string InviterUid;
	string InviterName;
	string InviteeUid;
	string InviteeName;
	int ExpiresAtMinute;
	int Status; // SM_DungeonInviteStatus
}

class SM_DungeonBooking
{
	string Id;
	string DungeonId;
	int DifficultyIndex;
	int Mode; // SM_DungeonMode
	int StartMinute;   // абсолютная минута запланированного старта
	int EndMinute;     // StartMinute + длительность слота
	int Status;        // SM_DungeonBookingStatus
	string LeaderUid;
	string LeaderName;
	ref array<string> MemberUids = new array<string>;
	ref array<string> MemberNames = new array<string>;
	bool FinderOpen;      // объявление в поиске спутников открыто для новых участников
	string FinderComment;
	int CreatedAtMinute;
	bool Warned; // предупреждение "забег скоро начнётся" уже отправлено

	bool HasMember(string uid)
	{
		return LeaderUid == uid || MemberUids.Find(uid) >= 0;
	}

	int PartySize()
	{
		return 1 + MemberUids.Count();
	}

	void RemoveMember(string uid)
	{
		int idx = MemberUids.Find(uid);
		if (idx < 0)
			return;
		MemberUids.Remove(idx);
		if (idx < MemberNames.Count())
			MemberNames.Remove(idx);
	}
}

class SM_DungeonPlayerRecord
{
	string Uid;
	string LastKnownName;
	ref array<string> UnlockedDungeonIds = new array<string>;
	int TempLockUntilMinute;
	bool PermaBanned;
	ref array<int> OffenseMinutes = new array<int>;
	ref map<string, int> DungeonCooldownUntilMinute = new map<string, int>;

	bool IsUnlocked(string dungeonId)
	{
		return UnlockedDungeonIds.Find(dungeonId) >= 0;
	}

	void Unlock(string dungeonId)
	{
		if (UnlockedDungeonIds.Find(dungeonId) < 0)
			UnlockedDungeonIds.Insert(dungeonId);
	}

	int GetCooldownUntil(string dungeonId)
	{
		if (!DungeonCooldownUntilMinute || !DungeonCooldownUntilMinute.Contains(dungeonId))
			return 0;
		return DungeonCooldownUntilMinute.Get(dungeonId);
	}

	void SetCooldownUntil(string dungeonId, int minute)
	{
		if (!DungeonCooldownUntilMinute)
			DungeonCooldownUntilMinute = new map<string, int>;
		DungeonCooldownUntilMinute.Set(dungeonId, minute);
	}
}

class SM_DungeonBookingsDB
{
	ref array<ref SM_DungeonBooking> Bookings = new array<ref SM_DungeonBooking>;
}

class SM_DungeonInvitesDB
{
	ref array<ref SM_DungeonInvite> Invites = new array<ref SM_DungeonInvite>;
}

class SM_DungeonPlayersDB
{
	ref array<ref SM_DungeonPlayerRecord> Players = new array<ref SM_DungeonPlayerRecord>;
}

// ---------------------------------------------------------------------------
// Всё, что ниже, существует только в оперативной памяти сервера на время
// активного забега и не сохраняется в JSON - при рестарте сервера активные
// забеги не восстанавливаются (что для нефазового, "физического" данжа и
// правильно: зона просто освобождается заново при старте менеджера).
// ---------------------------------------------------------------------------

class SM_DungeonRunParticipant
{
	string Uid;
	string Name;
	vector OriginalPosition;
	float OriginalYaw;
	bool ReadyForEvac;
	bool LeftEarly;
	bool Disconnected;
}

class SM_DungeonLootRoll
{
	string Id;
	string ItemClassName;
	int Quantity;
	ref array<string> EligibleUids = new array<string>;
	ref map<string, int> Choices = new map<string, int>;
	int SecondsRemaining;
	string WinnerUid;
	bool Resolved;
	bool WasTie;
}

class SM_DungeonRunState
{
	string RunId;
	string BookingId;
	string DungeonId;
	int DifficultyIndex;
	int Phase; // SM_DungeonRunPhase
	int PhaseSecondsRemaining;
	int RunTimeSecondsRemaining;
	ref array<ref SM_DungeonRunParticipant> Participants = new array<ref SM_DungeonRunParticipant>;
	ref array<EntityAI> SpawnedEntities = new array<EntityAI>;
	ref array<ref SM_DungeonLootRoll> ActiveLootRolls = new array<ref SM_DungeonLootRoll>;
	int EvacHoldSecondsRemaining;

	SM_DungeonRunParticipant GetParticipant(string uid)
	{
		foreach (SM_DungeonRunParticipant participant : Participants)
		{
			if (participant && participant.Uid == uid)
				return participant;
		}
		return NULL;
	}

	int CountActiveParticipants()
	{
		int count = 0;
		foreach (SM_DungeonRunParticipant participant : Participants)
		{
			if (participant && !participant.Disconnected && !participant.LeftEarly)
				count++;
		}
		return count;
	}
}

class SM_DungeonTicketLootEntry
{
	string ClassName;
	int Quantity;

	void SM_DungeonTicketLootEntry(string className = "", int quantity = 1)
	{
		ClassName = className;
		Quantity = quantity;
	}
}
