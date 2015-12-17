#include "StdAfx.h"
#include "FileLpd.h"

#include <iostream>
#include <fstream>
#include <locale.h>

using namespace std;

CFileLpd::CFileLpd(void)
{
	m_pLayer = NULL;
	m_nLayerCnt = 0;

	m_SectLineX = NULL;
	m_SectLineY = NULL;

	m_dOverlapSize = 0;
	m_strFilePath = L"";
}


CFileLpd::~CFileLpd(void)
{
	if(m_pLayer != NULL)
	{
		for(int nCnt=0; nCnt<m_nLayerCnt; nCnt++)
		{
			if(m_pLayer[nCnt].pBlock != NULL)
			{
				for(int nBlock=0; nBlock < m_pLayer[nCnt].nSectorCount; nBlock++)
				{
					if(m_pLayer[nCnt].pBlock[nBlock].pEntity != NULL)
					{
						delete[] m_pLayer[nCnt].pBlock[nBlock].pEntity;
						m_pLayer[nCnt].pBlock[nBlock].pEntity = NULL;
					}
				}
			
				delete[] m_pLayer[nCnt].pBlock;
				m_pLayer[nCnt].pBlock = NULL;
			}

			if(m_pLayer[nCnt].pBlockOrder != NULL)
			{
				delete[] m_pLayer[nCnt].pBlockOrder;
				m_pLayer[nCnt].pBlockOrder = NULL;
			}
		}	

		delete[] m_pLayer;
	}

	if(m_SectLineX != NULL)
		delete[] m_SectLineX;

	if(m_SectLineY != NULL)
		delete[] m_SectLineY;
}


BOOL CFileLpd::LoadLpdFile(CString strPath)
{
	char buf[100];
	CString str;
	int i, j, k;
	
	setlocale(LC_ALL,"Korean");

	ifstream file(strPath, ios::in);

	if(file.fail())
		return FALSE;

	//Layer mem 삭제
	if(m_pLayer != NULL)
	{
		for(int nCnt=0; nCnt<m_nLayerCnt; nCnt++)
		{
			if(m_pLayer[nCnt].pBlock != NULL)
			{
				for(int nBlock=0; nBlock < m_pLayer[nCnt].nSectorCount; nBlock++)
				{
					if(m_pLayer[nCnt].pBlock[nBlock].pEntity != NULL)
					{
						delete[] m_pLayer[nCnt].pBlock[nBlock].pEntity;
						m_pLayer[nCnt].pBlock[nBlock].pEntity = NULL;
					}
				}
			
				delete[] m_pLayer[nCnt].pBlock;
				m_pLayer[nCnt].pBlock = NULL;
			}

			if(m_pLayer[nCnt].pBlockOrder != NULL)
			{
				delete[] m_pLayer[nCnt].pBlockOrder;
				m_pLayer[nCnt].pBlockOrder = NULL;
			}
		}
		delete[] m_pLayer;
	}

	//거리 측정
	file >> buf; 
	file >> buf; 
	file >> buf; file >> buf;
	str.Format(L"%S",buf); // Inspection Count
	m_nInspectionCount = _ttoi(str);
	
	for(i = 0; i < m_nInspectionCount; i++)
	{
		file >> buf; 
		file >> buf; 
		file >> buf;
		str.Format(L"%S",buf); // Inspection PosX
		m_dInspectionPosX[i] = _ttof(str);
		file >> buf; 
		file >> buf; 
		str.Format(L"%S",buf); // Inspection PosY
		m_dInspectionPosY[i] = _ttof(str);
	}

	//CheckInpPos();수정
	file >> buf; file >> buf; 
	str.Format(L"%S",buf); // File Version
	m_nLPDVersion = _ttoi(str);

	file >> buf; file >> buf; 
	str.Format(L"%S",buf); // Block Size
	m_dSectorSize = _ttof(str);

	file >> buf; file >> buf;
	str.Format(L"%S",buf); // Tool Count
	m_nLayerCnt = _ttoi(str);

	file >> buf; file >> buf; 
	str.Format(L"%S",buf); // Entity All Count
	m_nEntityCount = _ttoi(str);
	
	file >> buf; file >> buf; 
	str.Format(L"%S",buf); // Block Count X
	m_nBlockCntX = _ttoi(str);
	
	file >> buf; file >> buf; 
	str.Format(L"%S",buf); // Block Count Y
	m_nBlockCntY = _ttoi(str);
		
	//Layer Mem 할당
	m_pLayer = new LAYER_DATA [m_nLayerCnt];
	
	for(j = 0; j < m_nLayerCnt; j ++)
	{
		int nEntityIndex = 0;

		file >> buf; file >> buf;
		str.Format(L"%S",buf); // Tool Name
		m_pLayer[j].strLayerName = str;

		file >> buf; file >> buf;
		str.Format(L"%S",buf); // Point Count
		m_pLayer[j].nEntityCount = _ttoi(str);

		//m_pLayer[j].pEntity = new ENTITY_DATA [m_pLayer[j].nEntityCount];
		
// 		file >> buf; file >> buf;
// 		str.Format(L"%S",buf); // Tool Color
		//m_pToolPath[j].iToolColor = _ttoi(str);

		file >> buf; file >> buf;
		str.Format(L"%S",buf); // Hole Size
		m_pLayer[j].dHoleSize= _ttof(str);

		file >> buf; file >> buf;
		str.Format(L"%S",buf); // Block Count
		m_pLayer[j].nSectorCount = _ttoi(str);

		//Block 할당
		m_pLayer[j].pBlock = new BLOCK_DATA[m_pLayer[j].nSectorCount];
		for(int nTemp=0; nTemp < m_pLayer[j].nSectorCount; nTemp++)
		{
			m_pLayer[j].pBlock[nTemp].pEntity = NULL;
			m_pLayer[j].pBlock[nTemp].nEntityCnt = 0;
		}
		
		//Sector Order 할당
		m_pLayer[j].pBlockOrder = new int[m_pLayer[j].nSectorCount];

		//Sector Order
		for(int nBlockNum=0; nBlockNum<m_pLayer[j].nSectorCount;nBlockNum++)
		{
			if(nBlockNum==0)
				file >> buf; 
			file >> buf;
			str.Format(L"%S",buf); // Block Order
			m_pLayer[j].pBlockOrder[nBlockNum] = _ttoi(str);
		}

		//Layer내 Sector 별 메모리 할당
		for(i = 0; i < m_pLayer[j].nSectorCount; i++)
		{
			file >> buf; file >> buf;
			str.Format(L"%S",buf); // Block Number
			
			file >> buf; file >> buf;
			str.Format(L"%S",buf); // Point Count
			int nEntityCnt = _ttoi(str);
			m_pLayer[j].pBlock[i].nEntityCnt = nEntityCnt;
			m_pLayer[j].pBlock[i].pEntity = new ENTITY_DATA[nEntityCnt];

			file >> buf; file >> buf;
			str.Format(L"%S",buf); // Block Position X
			m_pLayer[j].pBlock[i].fX = _ttof(str);
			
			file >> buf; file >> buf;
			str.Format(L"%S",buf); // Block Position Y
			m_pLayer[j].pBlock[i].fY = _ttof(str);
			
			for(k = 0; k < nEntityCnt; k++)
			{
				file >> buf; 
				str.Format(L"%S",buf); // Drill Type
				
				if(str == "P")
				{
					m_pLayer[j].pBlock[i].pEntity[k].nType = _ENTITY_POINT;

					file >> buf;
					str.Format(L"%S",buf); // Drill Point X
					m_pLayer[j].pBlock[i].pEntity[k].cX	= _ttof(str);

					file >> buf;
					str.Format(L"%S",buf); // Drill Point Y
					m_pLayer[j].pBlock[i].pEntity[k].cY	= _ttof(str);

				}
				else if (str == "L")
				{
					m_pLayer[j].pBlock[i].pEntity[k].nType = _ENTITY_LINE;

					file >> buf;
					str.Format(L"%S",buf); // Rout sX
					m_pLayer[j].pBlock[i].pEntity[k].sX	= _ttof(str);

					file >> buf;
					str.Format(L"%S",buf); // Rout sY
					m_pLayer[j].pBlock[i].pEntity[k].sY	= _ttof(str);

					file >> buf;
					str.Format(L"%S",buf); // Rout eX
					m_pLayer[j].pBlock[i].pEntity[k].eX	= _ttof(str);

					file >> buf;
					str.Format(L"%S",buf); // Rout eY
					m_pLayer[j].pBlock[i].pEntity[k].eY	= _ttof(str);
				}
			}
		}
	}

	if(m_nLPDVersion > 1)
	{
		if(m_SectLineX != NULL){
			delete[] m_SectLineX;
			m_SectLineX = NULL;
		}

		if(m_SectLineY != NULL){
			delete[] m_SectLineY;
			m_SectLineY = NULL;
		}

		file >> buf; file >> buf;
		str.Format(L"%S",buf); // SectRowCnt
		m_nBlockLineCntX = _ttoi(str);

		m_SectLineX = new LINE_DATA[m_nBlockLineCntX];

		for(i = 0; i<m_nBlockLineCntX; i++){

			file >> buf;
			str.Format(L"%S",buf); // SectRow sX
			m_SectLineX[i].sX = _ttof(str);

			file >> buf;
			str.Format(L"%S",buf); // SectRow sY
			m_SectLineX[i].sY = _ttof(str);

			file >> buf;
			str.Format(L"%S",buf); // SectRow eX
			m_SectLineX[i].eX = _ttof(str);

			file >> buf;
			str.Format(L"%S",buf); // SectRow eY
			m_SectLineX[i].eY = _ttof(str);

		}

		file >> buf; file >> buf;
		str.Format(L"%S",buf); // SectColCnt
		m_nBlockLineCntY = _ttoi(str);

		m_SectLineY = new LINE_DATA[m_nBlockLineCntY];

		for(i = 0; i<m_nBlockLineCntY; i++){

			file >> buf;
			str.Format(L"%S",buf); // SectCol sX
			m_SectLineY[i].sX = _ttof(str);

			file >> buf;
			str.Format(L"%S",buf); // SectCol sY
			m_SectLineY[i].sY = _ttof(str);

			file >> buf;
			str.Format(L"%S",buf); // SectCol eX
			m_SectLineY[i].eX = _ttof(str);

			file >> buf;
			str.Format(L"%S",buf); // SectCol eY
			m_SectLineY[i].eY = _ttof(str);

		}
	}

// 	file >> buf; file >> buf; if(file.eof()){ file.close(); return true; }
// 	str.Format(L"%S",buf); // File Path
// 	m_strFilePath = str;
	m_dOverlapSize = 0.0;
	m_dOffsetX = 0.0;
	m_dOffsetY = 0.0;
	while(!file.eof())
	{
		file >> buf; str.Format(L"%S",buf); // Overlap Size

		if(str == L"OverlapSize:")
		{
			file >> buf; str.Format(L"%S",buf); // Overlap Size
			m_dOverlapSize = _ttof(str);
		}

		if(str == L"OFFSET_X:")
		{
			file >> buf; str.Format(L"%S",buf); // Overlap Size
			m_dOffsetX = _ttof(str);
		}

		if(str == L"OFFSET_Y:")
		{
			file >> buf; str.Format(L"%S",buf); // Overlap Size
			m_dOffsetY = _ttof(str);
		}

	}

	//file >> buf; file >> buf; if(file.eof()){ file.close(); return true; }
	//str.Format(L"%S",buf); // Overlap Size
	//m_dOverlapSize = _ttof(str);
	

	file.close();

	return true;
}