/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include "layers.h"

#include "mapitems.h"

#include <engine/map.h>

CLayers::CLayers()
{
	Unload();
}

void CLayers::Init(class IKernel *pKernel)
{
	Unload();

	m_pMap = pKernel->RequestInterface<IMap>();
	m_pMap->GetType(MAPITEMTYPE_GROUP, &m_GroupsStart, &m_GroupsNum);
	m_pMap->GetType(MAPITEMTYPE_LAYER, &m_LayersStart, &m_LayersNum);

	for(int g = 0; g < NumGroups(); g++)
	{
		CMapItemGroup *pGroup = GetGroup(g);
		for(int l = 0; l < pGroup->m_NumLayers; l++)
		{
			CMapItemLayer *pLayer = GetLayer(pGroup->m_StartLayer + l);

			if(pLayer->m_Type == LAYERTYPE_TILES)
			{
				CMapItemLayerTilemap *pTilemap = reinterpret_cast<CMapItemLayerTilemap *>(pLayer);
				bool IsEntities = false;

				if(pTilemap->m_Flags & TILESLAYERFLAG_GAME)
				{
					m_pGameLayer = pTilemap;
					m_pGameGroup = pGroup;

					// make sure the game group has standard settings
					m_pGameGroup->m_OffsetX = 0;
					m_pGameGroup->m_OffsetY = 0;
					m_pGameGroup->m_ParallaxX = 100;
					m_pGameGroup->m_ParallaxY = 100;

					if(m_pGameGroup->m_Version >= 2)
					{
						m_pGameGroup->m_UseClipping = 0;
						m_pGameGroup->m_ClipX = 0;
						m_pGameGroup->m_ClipY = 0;
						m_pGameGroup->m_ClipW = 0;
						m_pGameGroup->m_ClipH = 0;
					}

					IsEntities = true;
					//break;
				}
				if(pTilemap->m_Flags & TILESLAYERFLAG_TELE)
				{
					if(pTilemap->m_Version <= 2)
					{
						pTilemap->m_Tele = *((int *)(pTilemap) + 15);
					}
					m_pTeleLayer = pTilemap;
					IsEntities = true;
				}
				if(pTilemap->m_Flags & TILESLAYERFLAG_SPEEDUP)
				{
					if(pTilemap->m_Version <= 2)
					{
						pTilemap->m_Speedup = *((int *)(pTilemap) + 16);
					}
					m_pSpeedupLayer = pTilemap;
					IsEntities = true;
				}
				if(pTilemap->m_Flags & TILESLAYERFLAG_FRONT)
				{
					if(pTilemap->m_Version <= 2)
					{
						pTilemap->m_Front = *((int *)(pTilemap) + 17);
					}
					m_pFrontLayer = pTilemap;
					IsEntities = true;
				}
				if(pTilemap->m_Flags & TILESLAYERFLAG_SWITCH)
				{
					if(pTilemap->m_Version <= 2)
					{
						pTilemap->m_Switch = *((int *)(pTilemap) + 18);
					}
					m_pSwitchLayer = pTilemap;
					IsEntities = true;
				}
				if(pTilemap->m_Flags & TILESLAYERFLAG_TUNE)
				{
					if(pTilemap->m_Version <= 2)
					{
						pTilemap->m_Tune = *((int *)(pTilemap) + 19);
					}
					m_pTuneLayer = pTilemap;
					IsEntities = true;
				}

				if(IsEntities)
				{ // Ensure default color for entities layers
					pTilemap->m_Color = CColor(255, 255, 255, 255);
				}
			}
		}
	}

	InitTilemapSkip();
}

void CLayers::InitBackground(class IMap *pMap)
{
	Unload();

	m_pMap = pMap;
	m_pMap->GetType(MAPITEMTYPE_GROUP, &m_GroupsStart, &m_GroupsNum);
	m_pMap->GetType(MAPITEMTYPE_LAYER, &m_LayersStart, &m_LayersNum);

	for(int g = 0; g < NumGroups(); g++)
	{
		CMapItemGroup *pGroup = GetGroup(g);
		for(int l = 0; l < pGroup->m_NumLayers; l++)
		{
			CMapItemLayer *pLayer = GetLayer(pGroup->m_StartLayer + l);

			if(pLayer->m_Type == LAYERTYPE_TILES)
			{
				CMapItemLayerTilemap *pTilemap = reinterpret_cast<CMapItemLayerTilemap *>(pLayer);

				if(pTilemap->m_Flags & TILESLAYERFLAG_GAME)
				{
					m_pGameLayer = pTilemap;
					m_pGameGroup = pGroup;

					// make sure the game group has standard settings
					m_pGameGroup->m_OffsetX = 0;
					m_pGameGroup->m_OffsetY = 0;
					m_pGameGroup->m_ParallaxX = 100;
					m_pGameGroup->m_ParallaxY = 100;

					if(m_pGameGroup->m_Version >= 2)
					{
						m_pGameGroup->m_UseClipping = 0;
						m_pGameGroup->m_ClipX = 0;
						m_pGameGroup->m_ClipY = 0;
						m_pGameGroup->m_ClipW = 0;
						m_pGameGroup->m_ClipH = 0;
					}
					//We don't care about tile layers.
				}
			}
		}
	}

	InitTilemapSkip();
}

void CLayers::Unload()
{
	m_GroupsNum = 0;
	m_GroupsStart = 0;
	m_LayersNum = 0;
	m_LayersStart = 0;

	m_pGameGroup = nullptr;
	m_pGameLayer = nullptr;
	m_pMap = nullptr;

	m_pTeleLayer = nullptr;
	m_pSpeedupLayer = nullptr;
	m_pFrontLayer = nullptr;
	m_pSwitchLayer = nullptr;
	m_pTuneLayer = nullptr;
}

void CLayers::InitTilemapSkip()
{
	for(int g = 0; g < NumGroups(); g++)
	{
		const CMapItemGroup *pGroup = GetGroup(g);

		for(int l = 0; l < pGroup->m_NumLayers; l++)
		{
			const CMapItemLayer *pLayer = GetLayer(pGroup->m_StartLayer + l);

			if(pLayer->m_Type == LAYERTYPE_TILES)
			{
				const CMapItemLayerTilemap *pTilemap = (CMapItemLayerTilemap *)pLayer;
				CTile *pTiles = (CTile *)m_pMap->GetData(pTilemap->m_Data);
				for(int y = 0; y < pTilemap->m_Height; y++)
				{
					for(int x = 1; x < pTilemap->m_Width;)
					{
						int SkippedX;
						for(SkippedX = 1; x + SkippedX < pTilemap->m_Width && SkippedX < 255; SkippedX++)
						{
							if(pTiles[y * pTilemap->m_Width + x + SkippedX].m_Index)
								break;
						}

						pTiles[y * pTilemap->m_Width + x].m_Skip = SkippedX - 1;
						x += SkippedX;
					}
				}
			}
		}
	}
}

CMapItemGroup *CLayers::GetGroup(int Index) const
{
	return static_cast<CMapItemGroup *>(m_pMap->GetItem(m_GroupsStart + Index));
}

CMapItemLayer *CLayers::GetLayer(int Index) const
{
	return static_cast<CMapItemLayer *>(m_pMap->GetItem(m_LayersStart + Index));
}
