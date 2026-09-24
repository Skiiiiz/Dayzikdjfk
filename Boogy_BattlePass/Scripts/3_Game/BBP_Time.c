// Работа с датами сезонов. Время хранится как "минуты с 2000-01-01 00:00"
// (помещается в int), с учётом смещения часового пояса из Settings.json.
class BBP_Time
{
	static const int EPOCH_DAYS = 10957; // DaysFromCivil(2000, 1, 1)

	// Алгоритм Howard Hinnant: количество дней с 1970-01-01.
	static int DaysFromCivil(int y, int m, int d)
	{
		if (m <= 2)
			y = y - 1;

		int era = y / 400;
		int yoe = y - era * 400;
		int mp = m + 9;
		if (m > 2)
			mp = m - 3;
		int doy = (153 * mp + 2) / 5 + d - 1;
		int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
		return era * 146097 + doe - 719468;
	}

	static void CivilFromDays(int z, out int y, out int m, out int d)
	{
		z = z + 719468;
		int era = z / 146097;
		int doe = z - era * 146097;
		int yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
		y = yoe + era * 400;
		int doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
		int mp = (5 * doy + 2) / 153;
		d = doy - (153 * mp + 2) / 5 + 1;
		if (mp < 10)
			m = mp + 3;
		else
			m = mp - 9;
		if (m <= 2)
			y = y + 1;
	}

	static int ToMinutes(int y, int mo, int d, int h, int mi)
	{
		int days = DaysFromCivil(y, mo, d) - EPOCH_DAYS;
		return days * 1440 + h * 60 + mi;
	}

	static int NowMinutes(int timezoneOffsetMinutes)
	{
		int year;
		int month;
		int day;
		int hour;
		int minute;
		int second;
		GetYearMonthDayUTC(year, month, day);
		GetHourMinuteSecondUTC(hour, minute, second);
		return ToMinutes(year, month, day, hour, minute) + timezoneOffsetMinutes;
	}

	// Номер дня (для ежедневных заданий).
	static int DayKey(int minutes)
	{
		return minutes / 1440;
	}

	// Номер недели, неделя начинается с понедельника (2000-01-03 — понедельник).
	static int WeekKey(int minutes)
	{
		return (minutes / 1440 + 5) / 7;
	}

	// "YYYY-MM-DD" или "YYYY-MM-DD HH:MM". Возвращает -1 при ошибке.
	static int Parse(string text)
	{
		text = text.Trim();
		if (text.Length() < 10)
			return -1;

		int y = text.Substring(0, 4).ToInt();
		int mo = text.Substring(5, 2).ToInt();
		int d = text.Substring(8, 2).ToInt();
		int h = 0;
		int mi = 0;
		if (text.Length() >= 16)
		{
			h = text.Substring(11, 2).ToInt();
			mi = text.Substring(14, 2).ToInt();
		}

		if (y < 2000 || mo < 1 || mo > 12 || d < 1 || d > 31 || h < 0 || h > 23 || mi < 0 || mi > 59)
			return -1;

		return ToMinutes(y, mo, d, h, mi);
	}

	static string Format(int minutes)
	{
		int days = minutes / 1440;
		int rest = minutes - days * 1440;
		int y;
		int m;
		int d;
		CivilFromDays(days + EPOCH_DAYS, y, m, d);
		int h = rest / 60;
		int mi = rest - h * 60;
		return y.ToString() + "-" + Pad2(m) + "-" + Pad2(d) + " " + Pad2(h) + ":" + Pad2(mi);
	}

	static string Pad2(int value)
	{
		if (value < 10)
			return "0" + value.ToString();
		return value.ToString();
	}

	// "12д 04ч", "3ч 15м", "45м"
	static string FormatDuration(int seconds)
	{
		if (seconds < 0)
			seconds = 0;

		int totalMinutes = seconds / 60;
		int days = totalMinutes / 1440;
		int hours = (totalMinutes - days * 1440) / 60;
		int mins = totalMinutes - days * 1440 - hours * 60;

		if (days > 0)
			return days.ToString() + "д " + Pad2(hours) + "ч";
		if (hours > 0)
			return hours.ToString() + "ч " + Pad2(mins) + "м";
		return mins.ToString() + "м";
	}
}
