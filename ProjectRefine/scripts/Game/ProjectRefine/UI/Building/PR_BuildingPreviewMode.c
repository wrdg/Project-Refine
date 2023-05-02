// Class for managing preview of assets

class PR_BuildingPreviewMode
{
	protected const float BUILDING_TRACE_LENGTH = 10.0;	// How far to trace from camera pos to find placement pos
	protected const int DIRECTION_CHANGE_STEPS = 24;	// Amount of steps to fully rotate an asset
	protected const ResourceName PREVIEW_PREFAB = "{1485D218DB14A25F}PrefabsEditable/System/E_Preview.et";
	protected const ResourceName PREVIEW_MATERIAL = "{A3DAAE3F27033DC1}UI/Building/BuildingPreview.emat";
	protected const ResourceName PREVIEW_MATERIAL_INVALID = "{0C74705225C528E8}UI/Building/BuildingPreviewInvalid.emat";
	
	protected SCR_BasePreviewEntity m_PreviewEntity;
	protected bool m_bOrientToSurface;
	protected int m_iDirection = 0; // Direction of preview
	
	void Activate(ResourceName prefabResourceName, bool orientToSurface)
	{
		if (prefabResourceName.IsEmpty())
			return;
		
		m_bOrientToSurface = orientToSurface;
		
		if (m_PreviewEntity)
			delete m_PreviewEntity;
		
		Resource prefabResource = Resource.Load(prefabResourceName);
		array<ref SCR_BasePreviewEntry> previewEntries = SCR_PrefabPreviewEntity.GetPreviewEntriesFromPrefab(prefabResource);
		
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		
		vector previewPos, surfaceNorm;
		CalculatePreviewPosition(previewPos, surfaceNorm);
		CalculatePreviewTransform(previewPos, surfaceNorm, spawnParams.Transform);
		spawnParams.Parent = null;
		spawnParams.Scale = 1.0;
		
		SCR_BasePreviewEntity previewEntity = SCR_BasePreviewEntity.SpawnPreview(previewEntries, PREVIEW_PREFAB, GetGame().GetWorld(), spawnParams, PREVIEW_MATERIAL);
		m_PreviewEntity = previewEntity;
		UpdatePreviewEntity();
	}
	
	void Deactivate()
	{
		if (m_PreviewEntity)
		{
			delete m_PreviewEntity;
		}
	}
	
	void Update(float timeSlice)
	{
		if (m_PreviewEntity)
			UpdatePreviewEntity();
	}
	
	void UpdatePreviewEntity()
	{
		vector transform[4];
		bool posValid;
		GetAndValidateTransform(transform, posValid);
		
		m_PreviewEntity.SetTransform(transform);
		
		ResourceName material;
		if (posValid)
			material = PREVIEW_MATERIAL;
		else
			material = PREVIEW_MATERIAL_INVALID;
		SCR_Global.SetMaterial(m_PreviewEntity, material, true);
		
		// Debugging
		int debugColor = Color.RED;
		if (posValid)
			debugColor = Color.GREEN;
		Shape.CreateSphere(debugColor, ShapeFlags.VISIBLE | ShapeFlags.ONCE, transform[3], 0.2);	
	}
	
	void CycleDirection(int dir)
	{
		if (dir > 0)
		{
			m_iDirection++;
			if (m_iDirection >= DIRECTION_CHANGE_STEPS)
				m_iDirection = 0;
		}
		else if (dir < 0)
		{
			m_iDirection--;
			if (m_iDirection < 0)
				m_iDirection = DIRECTION_CHANGE_STEPS - 1;
		}
	}
	
	// Returns preview transform and if the position is valid
	void GetAndValidateTransform(out vector outTransform[4], out bool outPosValid)
	{
		vector previewPos, surfaceNorm;
		bool posAttachedToSurface = CalculatePreviewPosition(previewPos, surfaceNorm);
			
		vector transform[4];
		CalculatePreviewTransform(previewPos, surfaceNorm, transform);
		
		bool posValid = posAttachedToSurface;
		if (posValid)
			posValid = posValid && ValidatePosition(transform);
		
		outPosValid = posValid;
		for (int i = 0; i < 4; i++)
			outTransform[i] = transform[i];
	}
	
	// Calculates position of preview
	protected bool CalculatePreviewPosition(out vector outPosition, out vector outNormal)
	{	
		CameraManager cameraMgr = GetGame().GetCameraManager();
		
		CameraBase camera = cameraMgr.CurrentCamera();
		
		vector cameraDir = camera.GetTransformAxis(2);
		vector cameraPos = camera.GetTransformAxis(3);
		
		// Trace params
		TraceParam tp = new TraceParam();
		tp.Start = cameraPos;
		tp.End = cameraPos + BUILDING_TRACE_LENGTH*cameraDir;
		tp.LayerMask = EPhysicsLayerPresets.Projectile;
		tp.Flags = TraceFlags.WORLD | TraceFlags.OCEAN; // TraceFlags.ENTS
		array<IEntity> excludeArray = { GetGame().GetPlayerController().GetControlledEntity() };
		tp.ExcludeArray = excludeArray;
		float traceResultLength = GetGame().GetWorld().TraceMove(tp, null);
			
		vector traceResultPos = cameraPos + traceResultLength * BUILDING_TRACE_LENGTH * cameraDir;
		outPosition = traceResultPos;
		
		// Return false if we are not looking at ground
		if (traceResultLength == 1.0)
		{
			outNormal = Vector(0, 1, 0);
			return false;
		}
		
		outNormal = tp.TraceNorm;
		return true;
	}
	
	// Calculates whole transform of preview according to current direction
	protected void CalculatePreviewTransform(vector position, vector surfaceNorm, out vector outTransform[4])
	{
		float direction_rad = m_iDirection * Math.PI2 / DIRECTION_CHANGE_STEPS;
		
		// Override surface normal if we don't orient to surface
		if (!m_bOrientToSurface)
			surfaceNorm = Vector(0, 1, 0);
		
		vector vDirUncorrected = Vector(Math.Sin(direction_rad), 0, Math.Cos(direction_rad));
		vector vSide = surfaceNorm * vDirUncorrected;
		vector vDir = vSide * surfaceNorm;
		
		outTransform[0] = vSide;
		outTransform[1] = surfaceNorm;
		outTransform[2] = vDir;
		outTransform[3] = position;
	}
	
	// Validates position, returns true if it's free
	protected bool ValidatePosition(vector transform[4])
	{
		return true;
	}
}