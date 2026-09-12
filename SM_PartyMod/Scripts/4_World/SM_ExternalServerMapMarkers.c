class SM_ExternalServerMapMarkers
{
	static void Upsert(string source, string id, string name, vector position, int color, string iconPaa)
	{
		SM_ClanManager manager = SM_ClanManager.Get();
		if (!manager)
			return;

		manager.UpsertExternalServerMapMarker(source, id, name, position, color, iconPaa);
	}

	static void Remove(string source, string id)
	{
		SM_ClanManager manager = SM_ClanManager.Get();
		if (!manager)
			return;

		manager.RemoveExternalServerMapMarker(source, id);
	}

	static void ClearSource(string source)
	{
		SM_ClanManager manager = SM_ClanManager.Get();
		if (!manager)
			return;

		manager.ClearExternalServerMapMarkers(source);
	}
}
