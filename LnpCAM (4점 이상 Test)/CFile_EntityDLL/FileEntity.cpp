#include "StdAfx.h"
#include "FileEntity.h"
#include <math.h>
#include "Matrix.h"

#define PHI 3.141592653589793

#define _DRILL	0
#define _LINE	1
#define _CIRCLE	2
#define _ARC	3

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CFileEntity::CFileEntity(void)
	: m_nVersion(4)
	, m_bRemoveBlank(FALSE)
	, m_bCenterAlign(FALSE)
	, m_b3StepMode(false)
	, m_nInspectionCnt(0)
{
	InitializeFileFolder();
	m_pLayer = NULL;
	m_iBlockSize = 40;
	m_lFilePointCount = 0;
	m_fTotalMinX = 0;
	m_fTotalMinY = 0;
	m_fTotalMaxX = 0;
	m_fTotalMaxY = 0;
	m_fOverlap	= 0;
	m_bOptimization = TRUE;
	m_bMirror = false;
	m_bReverse = false;
	m_bLoadStopFlag = false;
	m_iTotalProgress = 0;
	m_iProgressCount = 0;
	m_iLayerCount = 0;
	BlockFileOut = NULL;
	BlockFileIn = NULL;
	CuttingFileIn = NULL;
	EntityFileOut = NULL;
	JobFileOut = NULL;
	m_pBlockPos = NULL;

	m_SectLine.Row		=	NULL;
	m_SectLine.Col		=	NULL;
	
	m_nSelLayer = -1;

	m_bSectorApply = FALSE;

	for(int i = 0; i < 4; i++)
	{
		m_dInspectionPosX[i] = 0.0;
		m_dInspectionPosY[i] = 0.0;
	}

	m_strProgressMsg = "";
}


CFileEntity::~CFileEntity(void)
{
	InitializeLayerMemory();
}


int CFileEntity::LoadEntityFile(void)
{
	int i;			/** i local variable 01*/

	/* Initialize Layer buffer */
	InitializeLayerMemory();

	/* Make Layer informations from file */
	if(!SetBufferEntity())
		return 0;

	for(i=0;i<m_nLayerBufferSize;i++)
	{
		if(m_bLoadStopFlag == TRUE )
			return 0;

		/* Mirror Option. UI does not support this option */
		if(m_bMirror == TRUE )
		{
			/* Origin Entity 로 수정 필요*/
			if(!DoMirror(i, 2))
			{
				return 0;
			}
		}

		/* Reverse option. UI does not support this option */
		if(m_bReverse == TRUE )
		{
			/* Origin Entity 로 수정 필요*/
			if(!DoReverse(i))
			{
				return 0;
			}
		}
	}

	InitSectorLine();

	return 1;
	MakeBlockAllLayer();
	
	for(i=0;i<m_nLayerBufferSize; i++)
	{	
		if(!MakeBlock(i))				
			return 0;
		
		/* 1. Read data from Block Point file.
		2. Sort block data by block index.
		3. Save file */
		if(!MakeBlockSort(i) || m_bLoadStopFlag == TRUE)			
			return 0;

		/* Sort Block by stage path optimization */
		if(!SortBlock_StagePos(i) || m_bLoadStopFlag == TRUE)
			return 0;

		/* Make Block Information file */
		if(!SaveBlockInformation(i) || m_bLoadStopFlag == TRUE)		
			return 0;

		//Cutting Buffer

		/* Load Block data from sorted block data file */
		if(!LoadBlockData(i) || m_bLoadStopFlag == TRUE)		
			return 0;

		/* Optimization option */
		if(m_bOptimization == TRUE)	
		{
			if(!DoOptimization(i))
			{
				return 0;
			}
		}
	}
	

	//작업 번호에 따라 포인트를 정리함
	return 1;
}


int CFileEntity::GetLineEntity(CString& buf)
{
	char line[1000];		/** line local variable 01*/

	if (fscanf_s(EntityFileOut,"%s",line,100) == false) return 0;
	buf=line;
	buf.TrimLeft();
	buf.TrimRight();

	return 1;
}

void CFileEntity::InitializeFileFolder(void)
{
	CString TempStr;			/** TempStr local variable 01*/
	CString TempStr2;			/** TempStr2 local variable 02*/
	int TempLength;				/** TempLength local variable 03*/
	int TempNumber;				/** TempNumber local variable 04*/
	TCHAR folder[1000];			/** folder local variable 05*/
	////////////////// Entity 저장 디렉토리 만들기 ///////////////////////////
	GetModuleFileName(AfxGetInstanceHandle(), folder, 1000);

	TempStr = folder;
	TempLength = TempStr.GetLength();
	TempStr.MakeReverse();
	TempNumber = TempStr.Find(_T("\\"),0);
	TempStr = TempStr.Right(TempLength-TempNumber-1);
	TempStr.MakeReverse();

	m_strCoordFolder = TempStr+_T("\\Coord");
	CreateDirectory(m_strCoordFolder, NULL);

	m_strCoordFolder = m_strCoordFolder+_T("\\Entity");
	CreateDirectory(m_strCoordFolder, NULL);

	///////////////////////////////////////////////////////////

	////////////////// Common 디렉토리 만들기 ///////////////////////////
	GetModuleFileName(AfxGetInstanceHandle(), folder, 1000);

	TempStr2 = folder;
	TempLength = TempStr2.GetLength();
	TempStr2.MakeReverse();
	TempNumber = TempStr2.Find(_T("\\"),0);
	TempStr2 = TempStr2.Right(TempLength-TempNumber-1);
	TempStr2.MakeReverse();

	m_strCommonFolder = TempStr2+_T("\\Coord");
	CreateDirectory(m_strCommonFolder, NULL);

	m_strCommonFolder = m_strCommonFolder+_T("\\Comm");
	CreateDirectory(m_strCommonFolder, NULL);
	///////////////////////////////////////////////////////////
}


int CFileEntity::SetBufferEntity(void)
{
	char read_path[1000];			/** read_path local variable 01*/
	CString path;					/** path local variable 02*/
	CString str;					/** str local variable 03*/
	int i;							/** i local variable 04*/
	int j;							/** j local variable 05*/
	int layer_cnt;					/** layer_cnt local variable 06*/
	int line_cnt;					/** line_cnt local variable 07*/
	int temp_k = -1;				/** temp_k local variable 08*/
	//int temp_cnt;					/** temp_cnt local variable 09*/
	CString strLayerTemp, strLayerTemp2;
	int nOriginCnt;

	m_fTotalMinX = 9999999;
	m_fTotalMinY = 9999999;
	m_fTotalMaxX = -9999999;
	m_fTotalMaxY = -9999999;

	wsprintfA(read_path, "%S\\entity.dat", m_strCommonFolder); 

	if( (EntityFileOut =_fsopen(read_path, "r",_SH_DENYNO))==NULL)
		return 0;

	GetLineEntity(m_strCommandLine); /* Skip '[EntityInformation]' string */
	GetLineEntity(m_strCommandLine); /* Skip 'InspectionCount' string */
	GetLineEntity(m_strCommandLine); /* Get inspection count */
	m_nInspectionCnt = _ttoi(m_strCommandLine);
	for(i = 0 ; i < m_nInspectionCnt; i++)
	{
		GetLineEntity(m_strCommandLine);				/* Skip 'InspectionPoint(count)' string */
		GetLineEntity(m_strCommandLine);				/* Skip 'X:' string */
		GetLineEntity(m_strCommandLine);				/* Get X position */
		m_dInspectionPosX[i] = _ttof(m_strCommandLine);	
		GetLineEntity(m_strCommandLine);				/* Skip 'Y:' string */
		GetLineEntity(m_strCommandLine);				/* Get Y position */
		m_dInspectionPosY[i] = _ttof(m_strCommandLine);
	}
	GetLineEntity(m_strCommandLine);					/* Skip 'EntityLenght' string */
	GetLineEntity(m_strCommandLine);					/* Get entity length */
	m_lFilePointCount = _ttoi(m_strCommandLine);
	GetLineEntity(m_strCommandLine);					/* Skip 'ToolCount' string */
	GetLineEntity(m_strCommandLine);					/* Get Tool Count */
	m_iLayerCount = _ttoi(m_strCommandLine);

	SetTotalProgress();

	if(m_iLayerCount<1)
	{
		fclose(EntityFileOut);
		return 0;
	}

	/* Create Layer buffer & Intialize */
	m_pBlockPos = NULL;

// 	if(m_b3StepMode)
// 		m_nLayerBufferSize = m_iLayerCount + 2;
// 	else
		m_nLayerBufferSize  = m_iLayerCount;

	m_pLayer = new LAYER[m_nLayerBufferSize+2];

	for(i=0;i<m_iLayerCount;i++)
	{
		m_pLayer[i].m_nOptimizeType = _ENTITY_LINE;
		m_pLayer[i].fHoleSize = 0;
		m_pLayer[i].pCutBlock = NULL;
		m_pLayer[i].pEntity = NULL; // 구조체 포인터 변수 
		m_pLayer[i].pOriginEntity = NULL; // 구조체 포인터 변수 
		m_pLayer[i].pBlockOrder = NULL;
		m_pLayer[i].pInBlockEntityCount = NULL;

		GetLineEntity(m_strCommandLine);					/* Get 'Layer' String */
		GetLineEntity(m_strCommandLine);					/* Get Tool Number */
		GetLineEntity(m_strCommandLine);					/* Get Tool Name */
		m_pLayer[i].strLayerName = m_strCommandLine;		
		
		GetLineEntity(m_strCommandLine);					/* Get Entity Count in Tool */
		nOriginCnt = _ttoi(m_strCommandLine);
		m_pLayer[i].pOriginEntity = new Entity_Information[nOriginCnt];
		
		m_pLayer[i].iOriginCount = 0;

		GetLineEntity(m_strCommandLine);					/*Get Hole Size */
		m_pLayer[i].fHoleSize = _ttof(m_strCommandLine);
	}

	for(i=m_iLayerCount;i<m_iLayerCount+2;i++)
	{
		m_pLayer[i] = m_pLayer[m_iLayerCount-1];

		m_pLayer[i].pCutBlock = NULL;
		m_pLayer[i].pEntity = NULL; // 구조체 포인터 변수 
		m_pLayer[i].pOriginEntity = NULL; // 구조체 포인터 변수 
		m_pLayer[i].pBlockOrder = NULL;
		m_pLayer[i].pInBlockEntityCount = NULL;
		m_pLayer[i].pCutBlock = NULL;
		m_pLayer[i].pEntity = NULL; // 구조체 포인터 변수 
		m_pLayer[i].pOriginEntity = NULL; // 구조체 포인터 변수 
		m_pLayer[i].pBlockOrder = NULL;
		m_pLayer[i].pInBlockEntityCount = NULL;
		m_pLayer[i].pOriginEntity = NULL;
		
		CString strNameTemp;
		strNameTemp.Format(L"_%d", i-m_iLayerCount);
		m_pLayer[i].strLayerName += strNameTemp;		
	}

	GetLineEntity(m_strCommandLine);					/* Get Skip "%" */

	layer_cnt = 0;
	line_cnt = 0;

	/* Fill layer information */
	while(line_cnt<m_lFilePointCount)
	{
		m_iProgressCount++;
		m_strProgressMsg.Format(L"Entity 분석중(%d/%d)",line_cnt, m_lFilePointCount);

		if(m_bLoadStopFlag == TRUE)
		{
			fclose(EntityFileOut);
			return 0;
		}

		GetLineEntity(m_strCommandLine);
		str = m_strCommandLine;
		if((str == "DRILL")||(str == "POINT")||(str == "LINE")||(str == "CIRCLE")||(str == "ARC")||(str == "ELLIPSE")||
			(str == "D")||(str == "P")||(str == "L")||(str == "C")||(str == "A")||(str == "E"))
		{
			GetLineEntity(m_strCommandLine);
			BOOL temp = FALSE;

			//GetLayerName
			for(j=0;j<m_iLayerCount;j++)
			{
				if(m_bLoadStopFlag == TRUE)
				{
					fclose(EntityFileOut);
					return 0;
				}	

				if(m_strCommandLine == m_pLayer[j].strLayerName)
				{
					temp_k = j;
					break;
				}
			}
			
			m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].tempLine = NULL;
			m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].tempSize = 0;

			//POINT OR DRILL
			if(str == "DRILL" || str == "D") /* Parse Drill informations. Gbr, Drl */
			{
				m_pLayer[temp_k].m_nOptimizeType = _ENTITY_POINT;

				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].nType = _ENTITY_DRILL;
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].nColor = _ttoi(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].cX = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].cY = _ttof(m_strCommandLine);
				
				CheckMinMaxTotalPoint(
					m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].cX,
					m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].cY);
				m_pLayer[temp_k].iOriginCount++;
				line_cnt++;
			}
			else if(str == "POINT" || str == "P") /* Parse Drill Informations. Dxf */
			{
				m_pLayer[temp_k].m_nOptimizeType = _ENTITY_POINT;

				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].nType = _ENTITY_POINT;
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].nColor = _ttoi(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].cX = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].cY = _ttof(m_strCommandLine);
				
				CheckMinMaxTotalPoint(
					m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].cX,
					m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].cY);
				m_pLayer[temp_k].iOriginCount++;
				line_cnt++;
			}
			else if(str == "LINE" || str == "L") /* Parse Rout Informations. Dxf */
			{
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].nType = _ENTITY_LINE;
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].nColor = _ttoi(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].sX = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].sY = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].eX = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].eY = _ttof(m_strCommandLine);
				
  				CheckMinMaxTotalLine(
  					m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].sX,
  					m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].sY,
 					m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].eX,
 					m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].eY);
				m_pLayer[temp_k].iOriginCount++;
				line_cnt++;
			}
			else if(str == "CIRCLE" || str == "C") /* Parse Rout Informations. Dxf */
			{
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].nType = _ENTITY_CIRCLE;
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].nColor = _ttoi(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].cX = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].cY = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].rad = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].sX = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].sY = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].eX = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].eY = _ttof(m_strCommandLine);
				
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].sA = 0.0;
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].eA = 360.0;

				ArcToLine(temp_k, m_pLayer[temp_k].iOriginCount);

//				Arc 로 변경 후 내부에서 체크
// 				CheckMinMaxTotalCircle(
// 				 	m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].cX,
// 				 	m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].cY,
// 					m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].rad);

				m_pLayer[temp_k].iOriginCount++;
				line_cnt++;
			}
			else if(str == "ARC" || str == "A") /* Parse Rout Informations. Dxf */
			{
				//ARC부 처리
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].nType = _ENTITY_ARC;
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].nColor = _ttoi(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].cX = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].cY = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].rad = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].sA = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].eA = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].sX = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].sY = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].eX = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].eY = _ttof(m_strCommandLine);

				ArcToLine(temp_k, m_pLayer[temp_k].iOriginCount);

				m_pLayer[temp_k].iOriginCount++;
				line_cnt++;
			}
			else if(str == "ELLIPSE" || str == "E") /* Parse Rout Informations. Dxf */
			{
				//ARC부 처리
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].nType = _ENTITY_ELLIPSE;
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].nColor = _ttoi(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].cX = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].cY = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].sX = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].sY = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].rad = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].sA = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[temp_k].pOriginEntity[m_pLayer[temp_k].iOriginCount].eA = _ttof(m_strCommandLine);
				
				EllipseToLine(temp_k, m_pLayer[temp_k].iOriginCount);

				m_pLayer[temp_k].iOriginCount++;
				line_cnt++;
			}
		}
	}


	fclose(EntityFileOut);
	return 1;
}

void CFileEntity::InitializeLayerMemory(void)
{
	int i;				/** i local variable 01*/
	m_bLoadStopFlag = FALSE;
	m_iProgressCount = 0;

	if(m_pLayer!=NULL)
	{
		for(i=0;i<m_nLayerBufferSize;i++)
		{
 			if(m_pLayer[i].pCutBlock != NULL)
			{
				for(int j=0;j<m_iMaxBlockNumber;j++)
				{
					if(m_pLayer[i].pCutBlock[j].m_pEntity != NULL){
						delete [] m_pLayer[i].pCutBlock[j].m_pEntity;
						m_pLayer[i].pCutBlock[j].m_pEntity= NULL;
					}
				}
				delete [] m_pLayer[i].pCutBlock;
				m_pLayer[i].pCutBlock = NULL;

			}

			if(m_pLayer[i].pEntity != NULL)
			{
				delete[] m_pLayer[i].pEntity;
				m_pLayer[i].pEntity = NULL;
			}
			if(m_pLayer[i].pOriginEntity != NULL)
			{
				if( i < m_iLayerCount )
				{
					for(int j=0; j<m_pLayer[i].iOriginCount; j++)
					{
						if(m_pLayer[i].pOriginEntity[j].tempLine != NULL){
							delete[] m_pLayer[i].pOriginEntity[j].tempLine;
							m_pLayer[i].pOriginEntity[j].tempLine = NULL;
						}
					}
					delete[] m_pLayer[i].pOriginEntity;
					m_pLayer[i].pOriginEntity = NULL;
				}
			}
			if(m_pLayer[i].pInBlockEntityCount != NULL)
			{
				delete[] m_pLayer[i].pInBlockEntityCount;
				m_pLayer[i].pInBlockEntityCount = NULL;
			}
			if(m_pLayer[i].pBlockOrder != NULL)
			{
				delete[] m_pLayer[i].pBlockOrder;
				m_pLayer[i].pBlockOrder = NULL;
			}
		}
		delete[] m_pLayer;
		m_pLayer = NULL;
	}

	if(m_pBlockPos != NULL)
	{
		delete[] m_pBlockPos;
		m_pBlockPos = NULL;
	}

	for(int i = 0; i < 4; i++)
	{
		m_dInspectionPosX[i] = 0.0;
		m_dInspectionPosY[i] = 0.0;
	}

	DeleteSectorLineMem();

	m_nSelLayer = -1;
}


int CFileEntity::GetLayerCount(void)
{
	return m_iLayerCount;
}


CString CFileEntity::GetLayerName(int iLayerNumber)
{
	return m_pLayer[iLayerNumber].strLayerName;
}


double CFileEntity::GetLayerHoleSize(int iLayerNumber)
{
	return m_pLayer[iLayerNumber].fHoleSize;
}


void CFileEntity::SetLayerHoleSize(int iLayerNumber, double fHoleSize)
{
	m_pLayer[iLayerNumber].fHoleSize = fHoleSize;
}


int CFileEntity::GetLayerPointCount(int iLayerNumber)
{
	return m_pLayer[iLayerNumber].iEntityCount;
}


int CFileEntity::CheckMinMax(int iLayerNumber)
{
	long k;			/** k local variable 01*/

	m_pLayer[iLayerNumber].fMinX = 9999;m_pLayer[iLayerNumber].fMinY = 9999;
	m_pLayer[iLayerNumber].fMaxX = -9999;m_pLayer[iLayerNumber].fMaxY = -9999;

	for(k=0;k<m_pLayer[iLayerNumber].iEntityCount;k++)
	{
		m_iProgressCount++;
		m_strProgressMsg.Format(L"레이어%d-도면 영역 검사 중(%d/%d)",iLayerNumber, k, m_pLayer[iLayerNumber].iEntityCount);
		if(m_bLoadStopFlag == TRUE)
			return 0;
		CheckMinMaxPoint(iLayerNumber,k);
	}
	return 1;

}

int CFileEntity::MakeBlock(int iLayerNumber)
{
	long cnt=0;			/** cnt local variable 01*/
	long count=0;		/** count local variable 02*/
	char path[1000];	/** path local variable 03*/
	CString layer;		/** layer local variable 04*/
/*	int Block;			/ ** Block local variable 07* /*/
	int nEntityType;

	wsprintfA(path, "%S\\coord%d.sec", m_strCoordFolder,iLayerNumber); 
	if((BlockFileIn =_fsopen(path, "w",_SH_DENYNO))==NULL)
		return 0;

	//통합 Sector 정보
// 	if( m_pLayer[iLayerNumber].fMaxX >   9999 ||
// 		m_pLayer[iLayerNumber].fMinX < - 9999 ||
// 		m_pLayer[iLayerNumber].fMaxY >   9999 ||
// 		m_pLayer[iLayerNumber].fMinY < - 9999 )
// 	{
// 		AfxMessageBox(L"Unable to Convert File\r\nPlease Check Original File.(DRL, DXF, GBR)");
// 		return 0;
// 	}
// 
// 	m_pLayer[iLayerNumber].m_iBlockCountX = 
// 		(long)(m_pLayer[iLayerNumber].fMaxX-m_pLayer[iLayerNumber].fMinX) / (long)m_iBlockSize + 1;
// 	m_pLayer[iLayerNumber].m_iBlockCountY = 
// 		(long)(m_pLayer[iLayerNumber].fMaxY-m_pLayer[iLayerNumber].fMinY) / (long)m_iBlockSize + 1;
// 
// 	m_pLayer[iLayerNumber].m_iMaxBlockNumber = 
// 		m_pLayer[iLayerNumber].m_iBlockCountX *
// 		m_pLayer[iLayerNumber].m_iBlockCountY;
// 
// 	m_pLayer[iLayerNumber].pBlockPos = new BLOCK_INFO[m_pLayer[iLayerNumber].m_iMaxBlockNumber];

	int nOriginLayer;

	if(iLayerNumber >= m_iLayerCount)
		nOriginLayer = m_iLayerCount - 1;
	else
		nOriginLayer = iLayerNumber;

	int nLineCnt = 0;

	for(cnt=0; cnt<m_pLayer[nOriginLayer].iOriginCount; cnt++)
	{
		m_iProgressCount++;
		m_strProgressMsg.Format(L"레이어%d-섹터 생성 중(%d/%d)",iLayerNumber,cnt, m_pLayer[nOriginLayer].iOriginCount);

		if(m_bLoadStopFlag == TRUE)
			return 0;

		nEntityType = m_pLayer[nOriginLayer].pOriginEntity[cnt].nType;

		if(nEntityType == _ENTITY_LINE)
		{
			nLineCnt = nLineCnt + ExploidLine(iLayerNumber, cnt);
		}

		// Exploid Point
		else if(nEntityType == _ENTITY_DRILL || nEntityType == _ENTITY_POINT)
		{
			nLineCnt = nLineCnt + ExploidPoint(iLayerNumber, cnt);
		}
		
		//Exploid ARC
		else if(nEntityType == _ENTITY_ARC || nEntityType == _ENTITY_ELLIPSE )
		{
			for(int nTemp=0; nTemp<m_pLayer[nOriginLayer].pOriginEntity[cnt].tempSize; nTemp++)
			{
				if(m_pLayer[nOriginLayer].pOriginEntity[cnt].tempLine[nTemp].nType == _ENTITY_LINE)
				{
					nLineCnt = nLineCnt + ExploidTemp(iLayerNumber, cnt, nTemp);
				}
			}
		}

		//Exploid Circle
		else if(nEntityType == _ENTITY_CIRCLE)
		{
			for(int nTemp=0; nTemp<m_pLayer[nOriginLayer].pOriginEntity[cnt].tempSize; nTemp++)
			{
				if(m_pLayer[nOriginLayer].pOriginEntity[cnt].tempLine[nTemp].nType == _ENTITY_LINE)
				{
					nLineCnt = nLineCnt + ExploidTemp(iLayerNumber, cnt, nTemp);
				}
			}
		}
	}

	m_pLayer[iLayerNumber].iEntityCount = nLineCnt;

	fclose(BlockFileIn);

	return 1;
}


int CFileEntity::MakeBlockAllLayer()
{
	m_bSectorApply = TRUE;

	if( m_fTotalMaxX >   9999 ||
		m_fTotalMinX < - 9999 ||
		m_fTotalMaxY >   9999 ||
		m_fTotalMinY < - 9999 )
	{
		AfxMessageBox(L"Unable to Convert File\r\nPlease Check Original File.(DRL, DXF, GBR)");
		return 0;
	}

	m_iBlockCountX = m_SectLine.RowCnt - 1;
	m_iBlockCountY = m_SectLine.ColCnt - 1;

	m_iMaxBlockNumber = m_iBlockCountX * m_iBlockCountY;

	//Init m_pBlockPos
	if(m_pBlockPos != NULL)
	{
		delete[] m_pBlockPos;
		m_pBlockPos = NULL;
	}
	m_pBlockPos = new BLOCK_INFO[m_iMaxBlockNumber];
		
	int Sector_Num = 0;
	for( int i=0;i<m_iBlockCountY;i++)
	{
		for( int j=0;j<m_iBlockCountX;j++)
		{
			m_pBlockPos[Sector_Num].fLeft = (m_SectLine.Row[j].sx);
			m_pBlockPos[Sector_Num].fBottom = (m_SectLine.Col[i].sy);
			m_pBlockPos[Sector_Num].fRight = (m_SectLine.Row[j+1].ex);
			m_pBlockPos[Sector_Num].fTop = (m_SectLine.Col[i+1].ey);

			m_pBlockPos[Sector_Num].fX = (m_pBlockPos[Sector_Num].fLeft + m_pBlockPos[Sector_Num].fRight) / 2.0;
			m_pBlockPos[Sector_Num].fY = (m_pBlockPos[Sector_Num].fBottom + m_pBlockPos[Sector_Num].fTop) / 2.0;
			
			Sector_Num++;
		}
	}

	return 1;
}


int CFileEntity::MakeBlockSort(int iLayerNumber)
{
	long Block;			/** Block local variable 01*/
	long count;			/** count local variable 02*/
	long k;				/** k local variable 03*/
	char path[1000];	/** path local variable 04*/
	CString TempStr;	/** TempStr local variable 05*/

	Entity_Information *temp = new Entity_Information[m_pLayer[iLayerNumber].iEntityCount+1];
	memset( temp, 0x0, sizeof(Entity_Information)*(m_pLayer[iLayerNumber].iEntityCount+1));

	wsprintfA(path, "%S\\coord%d.sec", m_strCoordFolder,iLayerNumber); 
	if((BlockFileOut =_fsopen(path, "r",_SH_DENYNO))==NULL)
	{
		delete [] temp;
		return 0;
	}

	for(k=0; k<m_pLayer[iLayerNumber].iEntityCount;k++)
	{
		m_iProgressCount++;
		m_strProgressMsg.Format(L"레이어%d-섹터 분할 중(%d/%d)",iLayerNumber, k, m_pLayer[iLayerNumber].iEntityCount);
		if(m_bLoadStopFlag == TRUE)
		{
			delete [] temp;
			return 0;
		}
		GetLineBlockFile(TempStr);

		//Point
		if(TempStr == L"POINT" || TempStr == L"DRILL")
		{
			temp[k].nType = _ENTITY_POINT;
			GetLineBlockFile(TempStr);
			temp[k].iBlock = _ttoi(TempStr);
			GetLineBlockFile(TempStr);
			temp[k].cX = _ttof(TempStr);
			GetLineBlockFile(TempStr);
			temp[k].cY = _ttof(TempStr);
		}

		//Line
		else if(TempStr == L"LINE")
		{
			temp[k].nType = _ENTITY_LINE;
			GetLineBlockFile(TempStr);
			temp[k].iBlock = _ttoi(TempStr);
			GetLineBlockFile(TempStr);
			temp[k].sX = _ttof(TempStr);
			GetLineBlockFile(TempStr);
			temp[k].sY = _ttof(TempStr);
			GetLineBlockFile(TempStr);
			temp[k].eX = _ttof(TempStr);
			GetLineBlockFile(TempStr);
			temp[k].eY = _ttof(TempStr);
		}
	}
	fclose(BlockFileOut);

	//Delete;
	if(m_pLayer[iLayerNumber].pEntity!=NULL)
	{
		delete [] m_pLayer[iLayerNumber].pEntity; // free Block buffer
		m_pLayer[iLayerNumber].pEntity = NULL;
	}

	m_pLayer[iLayerNumber].pEntity = new Entity_Information[m_pLayer[iLayerNumber].iEntityCount+1]; // allocate memory 
	//memset( m_pLayer[iLayerNumber].pEntity, 0x0, sizeof(Entity_Information)*(m_pLayer[iLayerNumber].iEntityCount+1));

	for( Block=0, count=0; Block<m_iMaxBlockNumber; Block++)
	{
		m_iProgressCount++;
		m_strProgressMsg.Format(L"레이어%d-섹터 복사 중(%d/%d)",iLayerNumber, Block, m_iMaxBlockNumber);
		if(m_bLoadStopFlag == TRUE)
		{
			delete [] temp;
			return 0;
		}
		for(k=0;k<m_pLayer[iLayerNumber].iEntityCount;k++)
		{
			if(m_bLoadStopFlag == TRUE)
				return 0;

			if((long)Block==temp[k].iBlock)
			{
				m_pLayer[iLayerNumber].pEntity[count] = temp[k];
				count++;
			}
		}
	}

	if(temp != NULL)
	{
		delete [] temp; // free Block buffer
		temp = NULL;
	}

	if(m_pLayer[iLayerNumber].pInBlockEntityCount != NULL)
	{
		delete[] m_pLayer[iLayerNumber].pInBlockEntityCount;
		m_pLayer[iLayerNumber].pInBlockEntityCount = NULL;
	}
	m_pLayer[iLayerNumber].pInBlockEntityCount = new int[m_iMaxBlockNumber];
	memset( m_pLayer[iLayerNumber].pInBlockEntityCount, 0x0, sizeof(int)*m_iMaxBlockNumber);
	 
	// for Debugging 
	wsprintfA(path, "%S\\coord%d.sor", m_strCoordFolder,iLayerNumber); 
	if((BlockFileIn =_fsopen(path, "w",_SH_DENYNO))==NULL)
		return 0;

	for(k=0;k<m_pLayer[iLayerNumber].iEntityCount;k++)
	{
		m_iProgressCount++;
		m_strProgressMsg.Format(L"레이어%d-섹터 개체 판별 중(%d/%d)",iLayerNumber, k, m_pLayer[iLayerNumber].iEntityCount);
		if(m_bLoadStopFlag == TRUE)
			return 0;

		m_pLayer[iLayerNumber].pInBlockEntityCount[m_pLayer[iLayerNumber].pEntity[k].iBlock]++;

		if(m_pLayer[iLayerNumber].pEntity[k].nType == _ENTITY_POINT)
		{
			fprintf(BlockFileIn, "%d P %lf %lf\n",
				m_pLayer[iLayerNumber].pEntity[k].iBlock,	
				m_pLayer[iLayerNumber].pEntity[k].cX,
				m_pLayer[iLayerNumber].pEntity[k].cY);
		}
		else if(m_pLayer[iLayerNumber].pEntity[k].nType == _ENTITY_LINE)
		{
			fprintf(BlockFileIn, "%d L %lf %lf %lf %lf\n",
				m_pLayer[iLayerNumber].pEntity[k].iBlock,	
				m_pLayer[iLayerNumber].pEntity[k].sX,
				m_pLayer[iLayerNumber].pEntity[k].sY,
				m_pLayer[iLayerNumber].pEntity[k].eX,
				m_pLayer[iLayerNumber].pEntity[k].eY);
		}
	}
	fclose(BlockFileIn);

	return 1;
}

int CFileEntity::SaveBlockInformation(int iLayerNumber)
{
	char path[1000];		/** path local variable 01*/
	int i;					/** i local variable 02*/

	wsprintfA(path, "%S\\BlockInfo%d.sor", m_strCoordFolder,iLayerNumber); 
	if((BlockFileIn =_fsopen(path, "w",_SH_DENYNO))==NULL)
		return 0;

	fprintf(BlockFileIn, "BlockCount %d\n", m_iMaxBlockNumber);

	for(i=0;i<m_iMaxBlockNumber;i++)
	{
		m_iProgressCount++;
		m_strProgressMsg.Format(L"레이어%d-섹터 정보 저장 중(%d/%d)",iLayerNumber, i, m_iMaxBlockNumber);
		if(m_bLoadStopFlag == TRUE)
			return 0;
		fprintf(BlockFileIn, "BlockNumber %d PointCount %d Position %lf %lf\n", 
			i,
			m_pLayer[iLayerNumber].pInBlockEntityCount[i],
			m_pBlockPos[i].fX,
			m_pBlockPos[i].fY);
	}
	fclose(BlockFileIn);
	return 1;
}


void CFileEntity::CheckMinMaxPoint(int iLayerNumber, int iPointNumber)
{
	double cx;			/** cx local variable 01*/
	double cy;			/** cy local variable 02*/
	cx = m_pLayer[iLayerNumber].pEntity[iPointNumber].cX;
	cy = m_pLayer[iLayerNumber].pEntity[iPointNumber].cY;
	if(m_pLayer[iLayerNumber].fMinX > cx) m_pLayer[iLayerNumber].fMinX = cx;
	if(m_pLayer[iLayerNumber].fMaxX < cx) m_pLayer[iLayerNumber].fMaxX = cx;
	if(m_pLayer[iLayerNumber].fMinY > cy) m_pLayer[iLayerNumber].fMinY = cy;
	if(m_pLayer[iLayerNumber].fMaxY < cy) m_pLayer[iLayerNumber].fMaxY = cy;

}


void CFileEntity::SavePointData(int iBlockNumber, double fCX, double fCY)
{
	fwprintf(BlockFileIn, _T("POINT %d %lf %lf\n"),
		iBlockNumber,
		fCX,
		fCY);
}

void CFileEntity::SaveLineData(int iBlockNumber, double fSX, double fSY, double fEX, double fEY)
{
	fwprintf(BlockFileIn, _T("LINE %d %lf %lf %lf %lf\n"),
		iBlockNumber,
		fSX,
		fSY, 
		fEX, 
		fEY	);
}


int CFileEntity::GetLineBlockFile(CString& buf)
{
	char line[1000];		/** line local variable 01*/

	if (fscanf_s(BlockFileOut,"%s",line,100) == false) return 0;
	buf=line;
	buf.TrimLeft();
	buf.TrimRight();

	return 1;
}

int CFileEntity::SortBlock_StagePos(int iLayerNumber)
{
	//Init pCutBlock
	if(m_pLayer[iLayerNumber].pCutBlock != NULL)
	{
		delete[] m_pLayer[iLayerNumber].pCutBlock;
		m_pLayer[iLayerNumber].pCutBlock = NULL;
	}

	m_pLayer[iLayerNumber].pCutBlock = new BLOCK_INFO[m_iMaxBlockNumber];
	memset( m_pLayer[iLayerNumber].pCutBlock, 0x0, sizeof(BLOCK_INFO)*(m_iMaxBlockNumber));

	//Init pBlockOrder
	if(m_pLayer[iLayerNumber].pBlockOrder != NULL)
	{
		delete[] m_pLayer[iLayerNumber].pBlockOrder;
		m_pLayer[iLayerNumber].pBlockOrder = NULL;
	}
	m_pLayer[iLayerNumber].pBlockOrder = new int[m_iMaxBlockNumber];

	InitSectorOrder(iLayerNumber);

	//Sector Optimize

	//int i;				
	//int j;				
	//int k;			
	//int l;			

	//float tempX;		
	//float tempY;		
	//double max;			
	//double temp_gap;	

	/*
	if(m_pLayer[iLayerNumber].iRealBlockCount <= 2)
		return 1;

	
	if(m_bOptimization == FALSE)
		return 1;


	if ((m_pLayer[iLayerNumber].fMaxX - m_pLayer[iLayerNumber].fMinX)>(m_pLayer[iLayerNumber].fMaxY - m_pLayer[iLayerNumber].fMinY))
		max = m_pLayer[iLayerNumber].fMaxX - m_pLayer[iLayerNumber].fMinX;
	else
		max = m_pLayer[iLayerNumber].fMaxY - m_pLayer[iLayerNumber].fMinY;



	if(((int)max/m_iBlockSize)>5)
		temp_gap = 4.0;
	else if(((int)max/m_iBlockSize)>3)
		temp_gap = 2.0;
	else
		temp_gap = 1.0;

	int temp_MaxMove = (int)(max/temp_gap+0.5);		
	float sx;				
	float sy;				
	float ex;				
	float ey;				
	int min_Block=0;		
	double min_value;		
	double temp_distance;	
	int temp;

	for(i=0;i<m_pLayer[iLayerNumber].iRealBlockCount-1;i++)
	{
		m_iProgressCount++;

		if(m_bLoadStopFlag == TRUE)
			return 0;

		sx = m_pBlockPos[m_pLayer[iLayerNumber].pBlockOrder[i]].fX;
		sy = m_pBlockPos[m_pLayer[iLayerNumber].pBlockOrder[i]].fY;

		min_value = 99999999.99999999;

		for(j=i+1;j<m_pLayer[iLayerNumber].iRealBlockCount;j++)
		{
			if(m_bLoadStopFlag == TRUE)
				return 0;

			ex = m_pBlockPos[m_pLayer[iLayerNumber].pBlockOrder[j]].fX;
			ey = m_pBlockPos[m_pLayer[iLayerNumber].pBlockOrder[j]].fY;

			temp_distance = Calculate2PointDistance(sx,sy,ex,ey);

			if(temp_distance < min_value)
			{
				min_value = temp_distance;
				min_Block = j;
			}
			else if(temp_distance == min_value)
			{
				if(ey < m_pBlockPos[m_pLayer[iLayerNumber].pBlockOrder[min_Block]].fY)
				{
					min_value = temp_distance;
					min_Block = j;
				}
			}
		}

		if(min_Block != i+1)
		{
			temp = m_pLayer[iLayerNumber].pBlockOrder[i+1];
			m_pLayer[iLayerNumber].pBlockOrder[i+1] = m_pLayer[iLayerNumber].pBlockOrder[min_Block];
			m_pLayer[iLayerNumber].pBlockOrder[min_Block] = temp;
		}

		if(min_value>temp_MaxMove)
		{
		double temp_minvalue = 9999.99;	/ ** temp_minvalue local variable 18* /
		int temp_Block;					/ ** temp_Block local variable 19* /
		BOOL min_flag = FALSE;			/ ** min_flag local variable 20* /
		for(k=1;k<i;k++)
		{
		float temp_sx = m_pLayer[iLayerNumber].pBlockPos[m_pLayer[iLayerNumber].pBlockOrder[k]].fX;			/ ** temp_sx local variable 21* /
		float temp_sy = m_pLayer[iLayerNumber].pBlockPos[m_pLayer[iLayerNumber].pBlockOrder[k]].fY;			/ ** temp_sy local variable 22* /
		float temp_ex = m_pLayer[iLayerNumber].pBlockPos[m_pLayer[iLayerNumber].pBlockOrder[k+1]].fX;		/ ** temp_ex local variable 23* /
		float temp_ey = m_pLayer[iLayerNumber].pBlockPos[m_pLayer[iLayerNumber].pBlockOrder[k+1]].fY;		/ ** temp_ey local variable 24* /
		float temp_cx = m_pLayer[iLayerNumber].pBlockPos[m_pLayer[iLayerNumber].pBlockOrder[min_Block]].fX;	/ ** temp_cx local variable 25* /
		float temp_cy = m_pLayer[iLayerNumber].pBlockPos[m_pLayer[iLayerNumber].pBlockOrder[min_Block]].fY;	/ ** temp_cy local variable 26* /

		double temp_distance1 = Calculate2PointDistance(temp_sx,temp_sy,temp_cx,temp_cy);		/ ** temp_distance1 local variable 27* /
		double temp_distance2 = Calculate2PointDistance(temp_ex,temp_ey,temp_cx,temp_cy);		/ ** temp_distance2 local variable 28* /

		if((temp_distance1+temp_distance2)<temp_minvalue)
		{
		min_flag = TRUE;
		temp_Block = k;
		temp_minvalue = temp_distance1+temp_distance2;

		}
		}
		if(min_flag == TRUE)
		{
		temp = m_pLayer[iLayerNumber].pBlockOrder[min_Block];
		for(l=min_Block-1;l>temp_Block;l--)
		{
		m_pLayer[iLayerNumber].pBlockOrder[l+1] = m_pLayer[iLayerNumber].pBlockOrder[l];

		}
		m_pLayer[iLayerNumber].pBlockOrder[temp_Block+1] = temp;

		}
		else
		{
		temp = m_pLayer[iLayerNumber].pBlockOrder[min_Block];
		m_pLayer[iLayerNumber].pBlockOrder[min_Block] = m_pLayer[iLayerNumber].pBlockOrder[i+1];
		m_pLayer[iLayerNumber].pBlockOrder[i+1] = temp;
		}

		}
		else if(min_Block != i+1)
		{
		temp = m_pLayer[iLayerNumber].pBlockOrder[min_Block];
		m_pLayer[iLayerNumber].pBlockOrder[min_Block] = m_pLayer[iLayerNumber].pBlockOrder[i+1];
		m_pLayer[iLayerNumber].pBlockOrder[i+1] = temp;
		}
	}
	*/
	return 1;
}


double CFileEntity::Calculate2PointDistance(double fX1, double fY1, double fX2, double fY2)
{
	double number;			/** number local variable 01*/
	number= sqrt(pow(fX1-fX2,2) + pow(fY1-fY2,2));
	
	return number;
}


double CFileEntity::DoRound(double dNumber)
{
	dNumber =  floor(10000.*(dNumber + 0.00005))/10000. ;
	return dNumber;
}


CString CFileEntity::GetCoordFolder(void)
{
	return m_strCoordFolder;
}


void CFileEntity::SetOptimization(BOOL bFlag)
{
	m_bOptimization = bFlag;
}


void CFileEntity::SetMirror(BOOL bFlag)
{
	m_bMirror = bFlag;
}


void CFileEntity::SetReverse(BOOL bFlag)
{
	m_bReverse = bFlag;
}


int CFileEntity::LoadBlockData(int iLayerNumber)
{
	char path[1000];
	wsprintfA(path, "%S\\coord%d.sor", m_strCoordFolder,iLayerNumber); 
	if((CuttingFileIn =_fsopen(path, "r",_SH_DENYNO))==NULL)
		return 0;

	for(int i=0;i<m_pLayer[iLayerNumber].iRealBlockCount;i++)
	{
		m_iProgressCount++;
		m_strProgressMsg.Format(L"레이어%d-섹터 정보 로드 중(%d/%d)",iLayerNumber, i, m_pLayer[iLayerNumber].iRealBlockCount);

		GetLineEntity(m_strCommandLine); // Get Block Num

		int BlockNum = _ttoi(m_strCommandLine);
		int PointCount = m_pLayer[iLayerNumber].pInBlockEntityCount[BlockNum];

		m_pLayer[iLayerNumber].pCutBlock[BlockNum].m_pEntity= new Entity_Information[PointCount];
		m_pLayer[iLayerNumber].pCutBlock[BlockNum].lBlockNumber = BlockNum;

		for(int j=0;j<PointCount;j++)
		{
			if(j!=0){
				GetLineEntity(m_strCommandLine);
			}
			
			GetLineEntity(m_strCommandLine); // Get Entity type
			
			if(m_strCommandLine == "L")
			{
				m_pLayer[iLayerNumber].pCutBlock[BlockNum].m_pEntity[j].nType = _ENTITY_LINE;

				GetLineEntity(m_strCommandLine);
				m_pLayer[iLayerNumber].pCutBlock[BlockNum].m_pEntity[j].sX = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[iLayerNumber].pCutBlock[BlockNum].m_pEntity[j].sY = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[iLayerNumber].pCutBlock[BlockNum].m_pEntity[j].eX = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[iLayerNumber].pCutBlock[BlockNum].m_pEntity[j].eY = _ttof(m_strCommandLine);
			}
			else if(m_strCommandLine == "P")
			{
				m_pLayer[iLayerNumber].pCutBlock[BlockNum].m_pEntity[j].nType = _ENTITY_POINT;

				GetLineEntity(m_strCommandLine);
				m_pLayer[iLayerNumber].pCutBlock[BlockNum].m_pEntity[j].cX = _ttof(m_strCommandLine);
				GetLineEntity(m_strCommandLine);
				m_pLayer[iLayerNumber].pCutBlock[BlockNum].m_pEntity[j].cY = _ttof(m_strCommandLine);
			}
		}
	}

	fclose(CuttingFileIn);
	return 1;
}


int CFileEntity::GetLineCutting(CString& buf)
{
	char line[1000];		/** line local variable 01*/

	if (fscanf_s(CuttingFileIn,"%s",line,100) == false) return 0;
	buf=line;
	buf.TrimLeft();
	buf.TrimRight();

	return 1;
}

//수정
int CFileEntity::DoOptimization(int iLayerNumber)
{
	double MinDis;
	double nDis;
	double tempX, tempY;
	int nMin=0;

	double swap_x, swap_y;
	BOOL bSwapFlag;

	for(int i=0;i<m_iMaxBlockNumber;i++)
	{
		if(m_bLoadStopFlag == TRUE)
			return 0;
		//int BlockNum = m_pLayer[iLayerNumber].pCutBlock[i].lBlockNumber;
		int PointCount = m_pLayer[iLayerNumber].pInBlockEntityCount[i];
		int PointDouble = PointCount * PointCount;
		int nEnCnt = 0;

		//Point
		if(m_pLayer[iLayerNumber].m_nOptimizeType == _ENTITY_POINT)
		{
			for(int k = 0; k < PointCount - 1; k++)
			{
				if(m_bLoadStopFlag == TRUE)
					return 0;

				MinDis = 999999.999999;
				m_strProgressMsg.Format(L"레이어%d-개체 최적화 중. 블럭(%d/%d), 개체(%d/%d)",iLayerNumber, i, m_iMaxBlockNumber, nEnCnt, PointDouble);
				for(int l = k+1; l < PointCount; l++)
				{		
					nEnCnt++;
					m_iProgressCount++;
					

					if(m_bLoadStopFlag == TRUE)
						return 0;

					nDis = Calculate2PointDistance(
						m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[k].cX, 
						m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[k].cY, 
						m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[l].cX, 
						m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[l].cY );

					if(nDis < MinDis)
					{
						MinDis = nDis;
						nMin = l;
					}

					else if(nDis == MinDis)
					{
						if( m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[l].cX < 
							m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[nMin].cX ||
							m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[l].cY < 
							m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[nMin].cY)
						{
							MinDis = nDis;
							nMin = l;
						}
					}
				}

				if(k + 1 != nMin)
				{
					tempX = m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[k+1].cX;
					tempY = m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[k+1].cY;

					m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[k+1].cX = m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[nMin].cX;
					m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[k+1].cY = m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[nMin].cY;

					m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[nMin].cX = tempX;
					m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[nMin].cY = tempY;
				}
			}
		}

		//Line
		else if(m_pLayer[iLayerNumber].m_nOptimizeType == _ENTITY_LINE)
		{
			Entity_Information lineTemp;
			int iBefore, iNext;
			double Dis_A, Dis_B;
			double min;
			int num;

			for(iBefore = 0; iBefore < PointCount - 1; iBefore++)
			{
				min = 999999.999999;
				m_strProgressMsg.Format(L"레이어%d-개체 최적화 중. 블럭(%d/%d), 개체(%d/%d)",iLayerNumber, i, m_iMaxBlockNumber, nEnCnt, PointDouble);
				for(iNext = iBefore + 1; iNext < PointCount; iNext++ )
				{
					nEnCnt++;
					m_iProgressCount++;
					
							
					Dis_A = Calculate2PointDistance( 
						m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[iBefore].eX,
						m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[iBefore].eY,
						m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[iNext].sX,
						m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[iNext].sY);

					Dis_B = Calculate2PointDistance( 
						m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[iBefore].eX,
						m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[iBefore].eY,
						m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[iNext].eX,
						m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[iNext].eY);						

					nDis = (Dis_A > Dis_B) ? Dis_B : Dis_A;
												
					if(nDis < min)
					{
						min = nDis;
						num = iNext;

						bSwapFlag = (Dis_A > Dis_B) ? true : false;
					}

					if(min < 0.0000001)
						break;
				}

				//Swapping
				if(bSwapFlag)
				{
					if(m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[num].nType == _ENTITY_ARC){
						(m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[num].ang *=(-1));
					}

					swap_x = m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[num].sX;
					swap_y = m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[num].sY;

					m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[num].sX = m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[num].eX;
					m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[num].sY = m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[num].eY;

					m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[num].eX = swap_x;
					m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[num].eY = swap_y;
				}
						
				lineTemp = m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[iBefore+1];
				m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[iBefore+1] = m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[num];
				m_pLayer[iLayerNumber].pCutBlock[i].m_pEntity[num] = lineTemp;
			}
		}
	}
	return 1;
}


int CFileEntity::DoReverse(int iLayerNumber)
{
	for(int i=0;i<m_pLayer[iLayerNumber].iEntityCount;i++)
	{
		m_iProgressCount++;
		//m_strProgressMsg.Format(L"레이어%d-개체 최적화 중(%d/%d)",iLayerNumber, i, m_pLayer[iLayerNumber].iEntityCount);
		if(m_bLoadStopFlag == TRUE)
			return 0;
		double fX = m_pLayer[iLayerNumber].pEntity[i].sX;
		double fY = m_pLayer[iLayerNumber].pEntity[i].sY;
		m_pLayer[iLayerNumber].pEntity[i].sX = fY;
		m_pLayer[iLayerNumber].pEntity[i].sY = fX;
	}
	return 1;
}


int CFileEntity::DoMirror(int iLayerNumber, int nVersion)
{
	double cx = (m_fTotalMinX+m_fTotalMaxX);
	double tempX, tempY;

	for(int i=0;i<m_pLayer[iLayerNumber].iEntityCount;i++)
	{
		m_iProgressCount++;
		//m_strProgressMsg.Format(L"레이어%d-미러링 중(%d/%d)",iLayerNumber, i, m_pLayer[iLayerNumber].iEntityCount);
		if(m_bLoadStopFlag == TRUE)
			return 0;

		double fX = m_pLayer[iLayerNumber].pEntity[i].sX;
		//m_pLayer[iLayerNumber].pEntity[i].fX = cx - fX;

		if(nVersion == 2)
		{
			tempX = m_pLayer[iLayerNumber].pEntity[i].sX;
			tempY = m_pLayer[iLayerNumber].pEntity[i].sY;

			m_pLayer[iLayerNumber].pEntity[i].sX = tempY;
			m_pLayer[iLayerNumber].pEntity[i].sY = tempX;
		}
		if(nVersion == 3)
		{
			tempX = m_pLayer[iLayerNumber].pEntity[i].sX;
			tempY = m_pLayer[iLayerNumber].pEntity[i].sY;

			m_pLayer[iLayerNumber].pEntity[i].sX = -tempY;
			m_pLayer[iLayerNumber].pEntity[i].sY =  tempX;
		}
		if(nVersion == 4) // X Mirror
			m_pLayer[iLayerNumber].pEntity[i].sX = -m_pLayer[iLayerNumber].pEntity[i].sX;
		if(nVersion == 5)
		{
			tempX = m_pLayer[iLayerNumber].pEntity[i].sX;
			tempY = m_pLayer[iLayerNumber].pEntity[i].sY;

			m_pLayer[iLayerNumber].pEntity[i].sX = -tempY;
			m_pLayer[iLayerNumber].pEntity[i].sY = -tempX;
		}
		if(nVersion == 6) // X, Y Mirror
		{
			m_pLayer[iLayerNumber].pEntity[i].sX = -m_pLayer[iLayerNumber].pEntity[i].sX;
			m_pLayer[iLayerNumber].pEntity[i].sY = -m_pLayer[iLayerNumber].pEntity[i].sY;
		}
		if(nVersion == 7) // Y Mirror
			m_pLayer[iLayerNumber].pEntity[i].sY = -m_pLayer[iLayerNumber].pEntity[i].sY;
		if(nVersion == 8)
		{
			tempX = m_pLayer[iLayerNumber].pEntity[i].sX;
			tempY = m_pLayer[iLayerNumber].pEntity[i].sY;

			m_pLayer[iLayerNumber].pEntity[i].sX =  tempY;
			m_pLayer[iLayerNumber].pEntity[i].sY = -tempX;
		}

	}
	return 1;
}

void CFileEntity::CheckMinMaxTotalPoint(double fcX, double fcY)
{
	if(m_fTotalMinX > fcX) m_fTotalMinX = fcX;
	if(m_fTotalMaxX < fcX) m_fTotalMaxX = fcX;
	if(m_fTotalMinY > fcY) m_fTotalMinY = fcY;
	if(m_fTotalMaxY < fcY) m_fTotalMaxY = fcY;
}

void CFileEntity::CheckMinMaxTotalCircle(double fcX, double fcY, double fR)
{
	if(m_fTotalMinX > fcX-fR) m_fTotalMinX = fcX-fR;
	if(m_fTotalMaxX < fcX+fR) m_fTotalMaxX = fcX+fR;
	if(m_fTotalMinY > fcY-fR) m_fTotalMinY = fcY-fR;
	if(m_fTotalMaxY < fcY+fR) m_fTotalMaxY = fcY+fR;
}

void CFileEntity::CheckMinMaxTotalArc(double fcX, double fcY, double fR, double sA, double eA)
{
	if(m_fTotalMinX > fcX-fR) m_fTotalMinX = fcX-fR;
	if(m_fTotalMaxX < fcX+fR) m_fTotalMaxX = fcX+fR;
	if(m_fTotalMinY > fcY-fR) m_fTotalMinY = fcY-fR;
	if(m_fTotalMaxY < fcY+fR) m_fTotalMaxY = fcY+fR;
}

void CFileEntity::CheckMinMaxTotalLine(double fsX, double fsY, double feX, double feY)
{
	if(m_fTotalMinX > feX) m_fTotalMinX = feX;
	if(m_fTotalMaxX < feX) m_fTotalMaxX = feX;
	if(m_fTotalMinY > feY) m_fTotalMinY = feY;
	if(m_fTotalMaxY < feY) m_fTotalMaxY = feY;

	if(m_fTotalMinX > fsX) m_fTotalMinX = fsX;
	if(m_fTotalMaxX < fsX) m_fTotalMaxX = fsX;
	if(m_fTotalMinY > fsY) m_fTotalMinY = fsY;
	if(m_fTotalMaxY < fsY) m_fTotalMaxY = fsY;

}

int CFileEntity::SaveJobData(CString strFilepath)
{
	if( m_bLoadStopFlag == TRUE )
		return 0;

	char path[1000];		/** path local variable 01*/
	int i;

	int nLayerCnt = m_nLayerBufferSize;

	wsprintfA(path, "%S", strFilepath); 

	if((JobFileOut =_fsopen(path, "w",_SH_DENYNO))==NULL)
		return 0;

	fprintf(JobFileOut, "Start\n");
	fprintf(JobFileOut, "<JobInformation>\n");
	fprintf(JobFileOut, "\n");

	fprintf(JobFileOut, "InspectionCount %d\n", m_nInspectionCnt);

	for(i = 0; i < m_nInspectionCnt; i++)
	{													
		fprintf(JobFileOut, "InspectionPoint%d X: %lf Y: %lf\n", i+1, m_dInspectionPosX[i], m_dInspectionPosY[i]);
	}
	fprintf(JobFileOut, "\n");

	fprintf(JobFileOut, "Version %d\n", m_nVersion);
	fprintf(JobFileOut, "BlockSize %d\n", m_iBlockSize);
	fprintf(JobFileOut, "ToolCount %d\n", nLayerCnt);

	//WHole Entity Count
	int nEntityAllCnt = 0;

	for(i=0;i<nLayerCnt;i++)
	{
		nEntityAllCnt += m_pLayer[i].iEntityCount;
	}
	fprintf(JobFileOut, "EntityAllCnt %d\n", nEntityAllCnt);
	
	fprintf(JobFileOut, "BlockCountX %d\n", m_iBlockCountX);
	fprintf(JobFileOut, "BlockCountY %d\n", m_iBlockCountY);
		
	for(i=0;i<nLayerCnt;i++)
	{
		fprintf(JobFileOut, "\nToolName %S\n", m_pLayer[i].strLayerName);
		fprintf(JobFileOut, "ToolPointCount %d\n", m_pLayer[i].iEntityCount);
		//fprintf(JobFileOut, "ToolColor %d\n", m_pLayer[i].i_LayerColor);
		fprintf(JobFileOut, "ToolHoleSize %.4f\n", m_pLayer[i].fHoleSize);
		fprintf(JobFileOut, "BlockCount %d\n", m_iMaxBlockNumber);

		/* Block Order */
		int nBlockNum = 0;
		for(; nBlockNum<m_pLayer[i].iRealBlockCount;nBlockNum++)
		{
			if(nBlockNum==0)
				fprintf(JobFileOut, "BlockOrder ");
			fprintf(JobFileOut, "%d ", m_pLayer[i].pBlockOrder[nBlockNum]);

			if(nBlockNum%10==9)
				fprintf(JobFileOut, "\n           ");
		}
		for(; nBlockNum<m_iMaxBlockNumber; nBlockNum++)
		{
			fprintf(JobFileOut, "%d ", -1);

			if(nBlockNum%10==9)
				fprintf(JobFileOut, "\n           ");
		}

		fprintf(JobFileOut, "\n           ");

		for(int j=0;j<m_iMaxBlockNumber;j++)
		{
			m_iProgressCount++;
			//m_strProgressMsg.Format(L"레이어%d-섹터 정보 저장 중(%d/%d)",i, j, m_iMaxBlockNumber);
			//int BlockNum = m_pBlockPos[j].lBlockNumber;
			int BlockNum = j;
			int PointCount = m_pLayer[i].pInBlockEntityCount[j];

			fprintf(JobFileOut, "\nBlockNumber %d\n", BlockNum);
			fprintf(JobFileOut, "BlockPointCount %d\n", PointCount);
			fprintf(JobFileOut, "BlockPositionX %lf\n", m_pBlockPos[j].fX);
			fprintf(JobFileOut, "BlockPositionY %lf\n", m_pBlockPos[j].fY);
			int TempNum = BlockNum;

			for(int k=0;k<PointCount;k++)
			{
				if(m_pLayer[i].pCutBlock[j].m_pEntity[k].nType == _ENTITY_LINE)
				{
					fprintf(JobFileOut, "L %lf %lf %lf %lf\n", 
						m_pLayer[i].pCutBlock[j].m_pEntity[k].sX,
						m_pLayer[i].pCutBlock[j].m_pEntity[k].sY,
						m_pLayer[i].pCutBlock[j].m_pEntity[k].eX,
						m_pLayer[i].pCutBlock[j].m_pEntity[k].eY);
				}

				else if(m_pLayer[i].pCutBlock[j].m_pEntity[k].nType == _ENTITY_POINT)
				{
					fprintf(JobFileOut, "P %lf %lf\n", 
						m_pLayer[i].pCutBlock[j].m_pEntity[k].cX,
						m_pLayer[i].pCutBlock[j].m_pEntity[k].cY);
				}
				
				if( m_bLoadStopFlag == TRUE )
				{
					fclose(JobFileOut);
					return 0;
				}
			}
		}
	}

	fprintf(JobFileOut, "\nSectRowCnt: %d\n",m_SectLine.RowCnt);
	for(int nCnt=0; nCnt < m_SectLine.RowCnt; nCnt++)
		fprintf(JobFileOut, "%lf %lf %lf %lf\n",m_SectLine.Row[nCnt].sx, m_SectLine.Row[nCnt].sy, m_SectLine.Row[nCnt].ex, m_SectLine.Row[nCnt].ey);

	fprintf(JobFileOut, "\nSectColCnt: %d\n",m_SectLine.ColCnt);
	for(int nCnt=0; nCnt < m_SectLine.ColCnt; nCnt++)
		fprintf(JobFileOut, "%lf %lf %lf %lf\n",m_SectLine.Col[nCnt].sx, m_SectLine.Col[nCnt].sy, m_SectLine.Col[nCnt].ex, m_SectLine.Col[nCnt].ey);

	fprintf(JobFileOut, "\nBaseFile: %S\n", m_strBaseFile);
	fprintf(JobFileOut, "OverlapSize: %.3fmm\n", m_fOverlap);

	fprintf(JobFileOut, "OFFSET_X: %.3fmm\n", m_dSectorTotalMoveX);
	fprintf(JobFileOut, "OFFSET_Y: %.3fmm\n", m_dSectorTotalMoveY);
		
	fprintf(JobFileOut, "\nEnd\n");
	fprintf(JobFileOut, "0\n");

	fclose(JobFileOut);

	after = clock();

	double tResult = (double)(after - before) / CLOCKS_PER_SEC;

	m_strProgressMsg.Format(L"소요시간 %.3f초",tResult);

	return 1;

}
void CFileEntity::SetLoadStop(BOOL bStopFlag)
{
	m_bLoadStopFlag = bStopFlag;
}


void CFileEntity::SetProgress(int iNum)
{
	m_iTotalProgress = iNum;
}


int CFileEntity::GetProgressTotal(void)
{
	return m_iTotalProgress;
}


int CFileEntity::GetProgress(void)
{
	return m_iProgressCount;
}

CString CFileEntity::GetProgressMsg(void)
{
	return m_strProgressMsg;
}


void CFileEntity::SetTotalProgress(void)
{
	int num1 = 6;
	int	num2 = 4;
	float num3 = 0;
	if(m_bMirror == TRUE)	
		num1++;
	if(m_bReverse == TRUE)			
		num1++;
	if(m_bOptimization == TRUE)		
		num3 = num3 + 1.7f;

	m_iTotalProgress = (int)
						( m_lFilePointCount * num1 +
						m_nLayerBufferSize * m_iMaxBlockNumber * num2 +
						m_lFilePointCount * num3 );
}


void CFileEntity::SetBlockSize(int BlockSize)
{
	m_iBlockSize = BlockSize;
}


void CFileEntity::SetEntityVersion(int nVersion)
{
	m_nVersion = nVersion;
}


int CFileEntity::GetInspectionCnt(void)
{
	return m_nInspectionCnt;
}


double CFileEntity::GetInspectionPos(int nIndex, CString strAxis)
{
	double rtnValue;

	rtnValue = 0.0;

	if(strAxis == "X") rtnValue = m_dInspectionPosX[nIndex];
	else if(strAxis == "Y") rtnValue = m_dInspectionPosY[nIndex];

	return rtnValue;
}

int CFileEntity::DeleteSectorLineMem()
{
	if(m_SectLine.Row != NULL)
	{
		delete[] m_SectLine.Row;
		m_SectLine.Row = NULL;

		m_SectLine.RowCnt = 0;

		delete[] m_SectLine.Col;
		m_SectLine.Col = NULL;

		m_SectLine.ColCnt = 0;
	}
	return 1;
}

int CFileEntity::InitSectorLine()

{	
	//m_bRemoveBlank = TRUE;
	//m_bCenterAlign = TRUE;
	//Row가 X축, Col이 Y축으로 쓰였음. 주의!!
	DeleteSectorLineMem();

	int add_x=0,add_y=0;
	double k1,k2;

	if( DoRound(m_fTotalMaxX) == DoRound(m_fTotalMinX) )
		add_x++;

	if( DoRound(m_fTotalMaxY) == DoRound(m_fTotalMinY) )
		add_y++;

	k1 = Change_float(Change_um(m_fTotalMaxX-m_fTotalMinX))/m_iBlockSize;
	k2 = ((int)(Change_float(Change_um(m_fTotalMaxX-m_fTotalMinX))/m_iBlockSize));
	if((k1-k2)>0.0)
		add_x++;

	k1 = Change_float(Change_um(m_fTotalMaxY-m_fTotalMinY))/m_iBlockSize;
	k2 = ((int)(Change_float(Change_um(m_fTotalMaxY-m_fTotalMinY))/m_iBlockSize));
	if((k1-k2)>0.0)
		add_y++;

	m_SectLine.RowCnt = (int)(Change_float(Change_um(m_fTotalMaxX-m_fTotalMinX))/m_iBlockSize+add_x) + 1;
	m_SectLine.ColCnt = (int)(Change_float(Change_um(m_fTotalMaxY-m_fTotalMinY))/m_iBlockSize+add_y) + 1;

	m_SectLine.Row = new LINE[m_SectLine.RowCnt];
	m_SectLine.Col = new LINE[m_SectLine.ColCnt];

	double dCX_Sector;
	double dCY_Sector;
	double dCX_File;
	double dCY_File;

	dCX_Sector = m_fTotalMinX + ((m_SectLine.RowCnt-1) * m_iBlockSize) / 2;
	dCY_Sector = m_fTotalMinY + ((m_SectLine.ColCnt-1) * m_iBlockSize) / 2;
	dCX_File   = (m_fTotalMaxX + m_fTotalMinX) / 2;
	dCY_File   = (m_fTotalMaxY + m_fTotalMinY) / 2;

	double dOffsetX = dCX_File - dCX_Sector;
	double dOffsetY = dCY_File - dCY_Sector;
	
	int line_Row = 0;
	int line_Col = 0;

	double dTempSx, dTempSy, dTempEx, dTempEy;
	
	if(m_bRemoveBlank && !m_bCenterAlign)
	{
		dTempSx = m_fTotalMinX;
		dTempEx = m_fTotalMaxX;
		dTempSy = m_fTotalMinY;
		dTempEy = m_fTotalMaxY;
	}
	else
	{
		dTempSx = m_fTotalMinX;
		dTempEx = m_fTotalMinX + (m_SectLine.RowCnt-1)*m_iBlockSize;
		dTempSy = m_fTotalMinY;
		dTempEy = m_fTotalMinY + (m_SectLine.ColCnt-1)*m_iBlockSize;
	}
	
	if(m_bCenterAlign)
	{
		dTempSx += dOffsetX;
		dTempEx += dOffsetX;
		dTempSy += dOffsetY;
		dTempEy += dOffsetY;
	}
	else
	{
		dOffsetX = 0;
		dOffsetY = 0;
	}
	
	for( int i=0; i<m_SectLine.RowCnt; i++)
	{
		if(m_bRemoveBlank && i == 0 )
		{
			m_SectLine.Row[i].sx = m_fTotalMinX;
			m_SectLine.Row[i].ex = m_fTotalMinX;
			m_SectLine.Row[i].sy = m_fTotalMinY;
			m_SectLine.Row[i].ey = m_fTotalMaxY;
		}
		else if(m_bRemoveBlank && i == m_SectLine.RowCnt-1)
		{
			m_SectLine.Row[i].sx = m_fTotalMaxX;
			m_SectLine.Row[i].ex = m_fTotalMaxX;
			m_SectLine.Row[i].sy = m_fTotalMinY;
			m_SectLine.Row[i].ey = m_fTotalMaxY;
		}
		else if(m_bRemoveBlank)
		{
			m_SectLine.Row[i].sx = dTempSx + i * m_iBlockSize;
			m_SectLine.Row[i].ex = dTempSx + i * m_iBlockSize;
			m_SectLine.Row[i].sy = m_fTotalMinY;
			m_SectLine.Row[i].ey = m_fTotalMaxY;
		}
		else
		{
			m_SectLine.Row[i].sx = dTempSx + i * m_iBlockSize;
			m_SectLine.Row[i].ex = dTempSx + i * m_iBlockSize;
			m_SectLine.Row[i].sy = dTempSy;
			m_SectLine.Row[i].ey = dTempEy;
		}
	}

	for( int i=0; i<m_SectLine.ColCnt; i++)
	{
		if(m_bRemoveBlank && i == 0 )
		{
			m_SectLine.Col[i].sx = m_fTotalMinX;
			m_SectLine.Col[i].ex = m_fTotalMaxX;
			m_SectLine.Col[i].sy = m_fTotalMinY;
			m_SectLine.Col[i].ey = m_fTotalMinY;
		}
		else if(m_bRemoveBlank && i == m_SectLine.ColCnt-1)
		{
			m_SectLine.Col[i].sx = m_fTotalMinX;
			m_SectLine.Col[i].ex = m_fTotalMaxX;
			m_SectLine.Col[i].sy = m_fTotalMaxY;
			m_SectLine.Col[i].ey = m_fTotalMaxY;
		}
		else if(m_bRemoveBlank)
		{
			m_SectLine.Col[i].sx = m_fTotalMinX;
			m_SectLine.Col[i].ex = m_fTotalMaxX;
			m_SectLine.Col[i].sy = dTempSy + i * m_iBlockSize;
			m_SectLine.Col[i].ey = dTempSy + i * m_iBlockSize;
		}
		else
		{
			m_SectLine.Col[i].sx = dTempSx;
			m_SectLine.Col[i].ex = dTempEx;
			m_SectLine.Col[i].sy = dTempSy + i * m_iBlockSize;
			m_SectLine.Col[i].ey = dTempSy + i * m_iBlockSize;
		}
	}

	m_dSectorTotalMoveX = 0;
	m_dSectorTotalMoveY = 0;


















	// 기존
	/*
	double i;
	for(i=m_fTotalMinX; Change_um(i) <= Change_um(m_fTotalMaxX);i+=m_iBlockSize)
	{
		m_SectLine.Row[line_Row].sx = i + dOffsetX;
		m_SectLine.Row[line_Row].ex = i + dOffsetX;
		m_SectLine.Row[line_Row].sy = ;
		m_SectLine.Row[line_Row].ey = m_fTotalMinY + (m_SectLine.ColCnt-1)*m_iBlockSize + dOffsetY;

		if((Change_um(m_fTotalMaxX - m_fTotalMinX)) == 0)
		{
			line_Row++;
			m_SectLine.Row[line_Row].sx = i + m_iBlockSize + dOffsetX;
			m_SectLine.Row[line_Row].ex = i + m_iBlockSize + dOffsetX;
			m_SectLine.Row[line_Row].sy = m_fTotalMinY + dOffsetY;
			m_SectLine.Row[line_Row].ey = m_fTotalMinY + (m_SectLine.ColCnt-1)*m_iBlockSize + dOffsetY;
			break;
		}
		else if( Change_um(i + m_iBlockSize + dOffsetX) > Change_um(m_fTotalMaxX) 
			&& Change_um(i) != Change_um(m_fTotalMaxX))
		{
			line_Row++;

			m_SectLine.Row[line_Row].sx = i + m_iBlockSize + dOffsetX;
			m_SectLine.Row[line_Row].ex = i + m_iBlockSize + dOffsetX;
			m_SectLine.Row[line_Row].sy = m_fTotalMinY + dOffsetY;
			m_SectLine.Row[line_Row].ey = m_fTotalMinY + (m_SectLine.ColCnt-1)*m_iBlockSize + dOffsetY;

// 			m_SectLine.Row[line_Row].sx = m_fTotalMaxX;
// 			m_SectLine.Row[line_Row].ex = m_fTotalMaxX;
// 			m_SectLine.Row[line_Row].sy = m_fTotalMinY;
// 			m_SectLine.Row[line_Row].ey = m_fTotalMaxY;
			break;
		}
		line_Row++;
	}
	for(i = m_fTotalMinY; Change_um(i) <= Change_um(m_fTotalMaxY); i += m_iBlockSize)
	{
		m_SectLine.Col[line_Col].sx = m_fTotalMinX + dOffsetX;
		m_SectLine.Col[line_Col].ex = m_fTotalMinX + (m_SectLine.RowCnt-1)*m_iBlockSize + dOffsetX;
		m_SectLine.Col[line_Col].sy = i + dOffsetY;
		m_SectLine.Col[line_Col].ey = i + dOffsetY;

		if((Change_um(m_fTotalMaxY - m_fTotalMinY)) == 0)
		{
			line_Col++;
			m_SectLine.Col[line_Col].sx = m_fTotalMinX + dOffsetX;
			m_SectLine.Col[line_Col].ex = m_fTotalMinX + (m_SectLine.RowCnt-1)*m_iBlockSize + dOffsetX;
			m_SectLine.Col[line_Col].sy = i + m_iBlockSize + dOffsetY;
			m_SectLine.Col[line_Col].ey = i + m_iBlockSize + dOffsetY;

		}
		else if( Change_um(i+m_iBlockSize+dOffsetY) > Change_um(m_fTotalMaxY) 
			&& Change_um(i) != Change_um(m_fTotalMaxY))
		{
			line_Col++;

			m_SectLine.Col[line_Col].sx = m_fTotalMinX + dOffsetX;
			m_SectLine.Col[line_Col].ex = m_fTotalMinX + (m_SectLine.RowCnt-1)*m_iBlockSize + dOffsetX;
			m_SectLine.Col[line_Col].sy = i + m_iBlockSize + dOffsetY;
			m_SectLine.Col[line_Col].ey = i + m_iBlockSize + dOffsetY;

// 			m_SectLine.Col[line_Col].sx = m_fTotalMinX;
// 			m_SectLine.Col[line_Col].ex = m_fTotalMaxX;
// 			m_SectLine.Col[line_Col].sy = m_fTotalMaxY;
// 			m_SectLine.Col[line_Col].ey = m_fTotalMaxY;

		}
		line_Col++;
	}
	*/
	return 1;
}


int CFileEntity::ModifySectorLineCoord(int rowNum, int ColNum, double rowCoordChange, double colCoordChange)
{
	return 0;
}


int CFileEntity::AddSectorLine(int LineType)
{
	if(LineType == _ROW_LINE)
	{
		LINE *tempLine = new LINE[m_SectLine.RowCnt];

		for(int i=0; i< m_SectLine.RowCnt; i++)
		{
			tempLine[i] = m_SectLine.Row[i];
		}

		delete[] m_SectLine.Row;

		m_SectLine.Row = new LINE[m_SectLine.RowCnt + 1];
		
		m_SectLine.Row[0] = tempLine[0];
		m_SectLine.Row[m_SectLine.RowCnt] = tempLine[m_SectLine.RowCnt-1];

		double dSize = ( m_SectLine.Row[m_SectLine.RowCnt].sx - m_SectLine.Row[0].sx) / ( m_SectLine.RowCnt );

		for(int i=1; i<m_SectLine.RowCnt; i++)
		{
			m_SectLine.Row[i].sx = m_SectLine.Row[0].sx + dSize*i;
			m_SectLine.Row[i].ex = m_SectLine.Row[0].ex + dSize*i;
			m_SectLine.Row[i].sy = m_SectLine.Row[0].sy;
			m_SectLine.Row[i].ey = m_SectLine.Row[0].ey;
		}

// 		for(int i=0; i< m_SectLine.RowCnt; i++)
// 		{
// 			m_SectLine.Row[i] = tempLine[i];
// 		}
// 		m_SectLine.Row[m_SectLine.RowCnt] = m_SectLine.Row[m_SectLine.RowCnt-1];
// 
// 		m_SectLine.Row[m_SectLine.RowCnt-1].sx = ( m_SectLine.Row[m_SectLine.RowCnt-2].sx + m_SectLine.Row[m_SectLine.RowCnt].sx ) / 2;
// 		m_SectLine.Row[m_SectLine.RowCnt-1].ex = ( m_SectLine.Row[m_SectLine.RowCnt-2].ex + m_SectLine.Row[m_SectLine.RowCnt].ex ) / 2;

		m_SectLine.RowCnt++;

		delete[] tempLine;
	}
	else if(LineType == _COL_LINE)
	{
		LINE *tempLine = new LINE[m_SectLine.ColCnt];

		for(int i=0; i< m_SectLine.ColCnt; i++)
		{
			tempLine[i] = m_SectLine.Col[i];
		}

		delete[] m_SectLine.Col;

		m_SectLine.Col = new LINE[m_SectLine.ColCnt + 1];
		
		m_SectLine.Col[0] = tempLine[0];
		m_SectLine.Col[m_SectLine.ColCnt] = tempLine[m_SectLine.ColCnt-1];

		double dSize = ( m_SectLine.Col[m_SectLine.ColCnt].sy - m_SectLine.Col[0].sy) / ( m_SectLine.ColCnt );

		for(int i=1; i<m_SectLine.ColCnt; i++)
		{
			m_SectLine.Col[i].sy = m_SectLine.Col[0].sy + dSize*i;
			m_SectLine.Col[i].ey = m_SectLine.Col[0].ey + dSize*i;
			m_SectLine.Col[i].sx = m_SectLine.Col[0].sx;
			m_SectLine.Col[i].ex = m_SectLine.Col[0].ex;
		}


// 		for(int i=0; i< m_SectLine.ColCnt; i++)
// 		{
// 			m_SectLine.Col[i] = tempLine[i];
// 		}
// 		m_SectLine.Col[m_SectLine.ColCnt] = m_SectLine.Col[m_SectLine.ColCnt-1];
// 
// 		m_SectLine.Col[m_SectLine.ColCnt-1].sy = ( m_SectLine.Col[m_SectLine.ColCnt-2].sy + m_SectLine.Col[m_SectLine.ColCnt].sy ) / 2;
// 		m_SectLine.Col[m_SectLine.ColCnt-1].ey = ( m_SectLine.Col[m_SectLine.ColCnt-2].ey + m_SectLine.Col[m_SectLine.ColCnt].ey ) / 2;

		m_SectLine.ColCnt++;

		delete[] tempLine;
	}

	return 1;
}

int CFileEntity::DeleteSectorLine(int LineType)
{
	int i;

	if(LineType == _ROW_LINE)
	{
		LINE *tempLine = new LINE[m_SectLine.RowCnt];
		
		for(i=0; i< m_SectLine.RowCnt; i++)
		{
			tempLine[i] = m_SectLine.Row[i];
		}
		tempLine[i-2] = tempLine[i-1];

		delete[] m_SectLine.Row;

		m_SectLine.RowCnt = m_SectLine.RowCnt -1;
		m_SectLine.Row = new LINE[m_SectLine.RowCnt];
		
		for(int i=0; i< m_SectLine.RowCnt; i++)
		{
			m_SectLine.Row[i] = tempLine[i];
		}

		delete[] tempLine;
	}
	else if(LineType == _COL_LINE)
	{
		LINE *tempLine = new LINE[m_SectLine.ColCnt];

		for(i=0; i< m_SectLine.ColCnt; i++)
		{
			tempLine[i] = m_SectLine.Col[i];
		}
		tempLine[i-2] = tempLine[i-1];

		delete[] m_SectLine.Col;

		m_SectLine.ColCnt = m_SectLine.ColCnt -1;
		m_SectLine.Col = new LINE[m_SectLine.ColCnt];
		
		for(int i=0; i< m_SectLine.ColCnt; i++)
		{
			m_SectLine.Col[i] = tempLine[i];
		}
		
		delete[] tempLine;
	}
	return 1;
}

int CFileEntity::Change_um(double val)
{
	//소수점 유효자리 6재짜리까지.

	unsigned long x = (unsigned long)(val * 1000000.0);
	return (unsigned int)(val * 1000000.0);
}


double CFileEntity::Change_float(int num)
{
	//Change Um 복귀
	return num / 1000000.0;
}

int CFileEntity::ExploidPoint(int nLayerNum, int nEntityNum)
{
	int nTempX = Change_um(m_pLayer[nLayerNum].pOriginEntity[nEntityNum].cX);
	int nTempY = Change_um(m_pLayer[nLayerNum].pOriginEntity[nEntityNum].cY);

	double TempX = m_pLayer[nLayerNum].pOriginEntity[nEntityNum].cX;
	double TempY = m_pLayer[nLayerNum].pOriginEntity[nEntityNum].cY;

	int nLineCnt = 0;

	for(int i=0; i< m_iMaxBlockNumber; i++)
	{
		if(m_bLoadStopFlag == TRUE){
			break;
		}

// 		if(m_pLayer[nLayerNum].pOriginEntity[nEntityNum].bUsedFlag == TRUE)
// 			break;

		if( (nTempX >= Change_um(m_pBlockPos[i].fLeft)) && 
			(nTempY >= Change_um(m_pBlockPos[i].fBottom)) && 
			(nTempX <= Change_um(m_pBlockPos[i].fRight)) &&
			(nTempY <= Change_um(m_pBlockPos[i].fTop) ))
		{
			//m_pLayer[nLayerNum].
			//Block = i;
			SavePointData(i, TempX, TempY);
			//m_pLayer[nLayerNum].pOriginEntity[nEntityNum].bUsedFlag = TRUE;
			nLineCnt++;
			break;
		}
	}
	if(nLineCnt == 0)
		int x =0;

	return nLineCnt;
}


int CFileEntity::ExploidLine(int nLayerNum, int nEntityNum)
{
	int nLineCnt = 0;

	int sect, i, ret;
	double sx, sy, ex, ey;
	double x, y;
	int dummy, ccnt=0;
	CString layer;

	double add_sx,add_sy,add_ex,add_ey,add_x, add_y,Line_Angle;
	double sect_sx, sect_sy, sect_ex, sect_ey;

	while(ccnt!=100)
	{
		int nOriginLayer;

		if(nLayerNum >= m_iLayerCount)
			nOriginLayer = m_iLayerCount -1;
		else
			nOriginLayer = nLayerNum;

		if(ccnt!=200)
		{
			sx = m_pLayer[nOriginLayer].pOriginEntity[nEntityNum].sX;
			sy = m_pLayer[nOriginLayer].pOriginEntity[nEntityNum].sY;
			ex = m_pLayer[nOriginLayer].pOriginEntity[nEntityNum].eX;
			ey = m_pLayer[nOriginLayer].pOriginEntity[nEntityNum].eY;
		}

		sect = 0;
		ccnt = 0;
		
		for( i=0; i<m_iMaxBlockNumber; i++)
		{
			if(m_b3StepMode && nLayerNum >= m_iLayerCount)
			{
				double dShift;

				if(nLayerNum == m_iLayerCount)
					dShift = 0.2;
				else if(nLayerNum == m_iLayerCount +1)
					dShift = -0.2;
													
				//첫 X열
				if( i % m_iBlockCountX == 0)				 sect_sx = m_pBlockPos[i].fLeft;
				else										 sect_sx = m_pBlockPos[i].fLeft	+ dShift;				

				//첫 Y열
				if( i < m_iMaxBlockNumber / m_iBlockCountY ) sect_sy = m_pBlockPos[i].fBottom;
				else										 sect_sy = m_pBlockPos[i].fBottom + dShift;
				
				//마지막 X열
				if( i % m_iBlockCountX == m_iBlockCountX-1 ) sect_ex = m_pBlockPos[i].fRight;
				else						 			     sect_ex = m_pBlockPos[i].fRight + dShift;

				//마지막 Y열
				if( i / m_iBlockCountX >= m_iBlockCountY-1 ) sect_ey = m_pBlockPos[i].fTop;
				else										 sect_ey = m_pBlockPos[i].fTop + dShift;
			}
			else
			{
				sect_sx = m_pBlockPos[i].fLeft;
				sect_sy = m_pBlockPos[i].fBottom;
				sect_ex = m_pBlockPos[i].fRight;
				sect_ey = m_pBlockPos[i].fTop;
			}

			if( ( (Change_um(sx)>=Change_um(sect_sx)) && (Change_um(sx)<=Change_um(sect_ex)) ) &&
				( (Change_um(ex)>=Change_um(sect_sx)) && (Change_um(ex)<=Change_um(sect_ex)) ) &&
				( (Change_um(sy)>=Change_um(sect_sy)) && (Change_um(sy)<=Change_um(sect_ey)) ) &&
				( (Change_um(ey)>=Change_um(sect_sy)) && (Change_um(ey)<=Change_um(sect_ey)) ) )
			{
				ccnt = 100;
				dummy = 0;
				nLineCnt++;
				SaveLineData(i ,sx,sy,ex,ey);
				break;
			}
			else   // sector를 벗어남
			{
				if( ((Change_um(sx)>=Change_um(sect_sx)) && (Change_um(sy)>=Change_um(sect_sy))) &&
					((Change_um(sx)<=Change_um(sect_ex)) && (Change_um(sy)<=Change_um(sect_ey))) )
				{
					dummy = 1; // 시작점이 Sector안에 있을경우 
					ret = SCAN_FindIntersection( (double)sx,(double)sy,(double)ex,(double)ey,
						                         (double)(sect_sx),(double)(sect_ex),(double)(sect_sy),(double)(sect_ey),
						&x,&y, dummy);
					if(ret)
					{
						nLineCnt++;
						//sect 를 나눈 선분에 길이 양쪽으로 증가
						Line_Angle = CalculateAngle2Point(sx,sy,x,y);
						Line_Angle=Line_Angle/180*PHI;
						add_sx=sx;//-0.1*cos(Line_Angle);
						add_sy=sy;//-0.1*sin(Line_Angle);
						add_x=x;
						add_y=y;
						add_x=x+m_fOverlap*cos(Line_Angle);
						add_y=y+m_fOverlap*sin(Line_Angle);
						SaveLineData(i,add_sx,add_sy,add_x,add_y);
						sx = x;
						sy = y;
						if(ret==1) ccnt=200;
					}
				}
				else if( ((Change_um(ex)>=Change_um(sect_sx)) && (Change_um(ey)>=Change_um(sect_sy))) &&
					((Change_um(ex)<=Change_um(sect_ex)) && (Change_um(ey)<=Change_um(sect_ey))) )
				{
					dummy = 2; // 끝점이 Sector안에 있을경우
					ret = SCAN_FindIntersection( (double)sx,(double)sy,(double)ex,(double)ey,
						(double)(sect_sx),(double)(sect_ex),(double)(sect_sy),(double)(sect_ey),
						&x,&y, dummy);
					if(ret)
					{
						nLineCnt++;
						Line_Angle = CalculateAngle2Point(x,y,ex,ey);

						if(Line_Angle > 89.999999)
							Line_Angle = Line_Angle;

						if(Line_Angle >89.999999 && Line_Angle < 90.000001)
							Line_Angle = 270;

						

						Line_Angle=Line_Angle/180*PHI;

						add_x=x;
						add_y=y;
						add_x=x-m_fOverlap*cos(Line_Angle);
						add_y=y-m_fOverlap*sin(Line_Angle);
						add_ex=ex;//+0.1*cos(Line_Angle);
						add_ey=ey;//+0.1*sin(Line_Angle);
						SaveLineData(i,add_x,add_y,add_ex,add_ey);
						ex = x;
						ey = y;
						if(ret==1) ccnt=200;
					}
				}
			}
			if(ccnt==200)
			{
				nEntityNum;
				break;
			}
			if(ccnt==100)
				break;
		}
	}
	return nLineCnt;
}


int CFileEntity::ExploidTemp(int nLayerNum, int nEntityNum, int nTemp)
{
	int nLineCnt = 0;

	int sect, i, ret;
	double sx, sy, ex, ey;
	double x, y;
	int dummy, ccnt=0;
	CString layer;

	double add_sx,add_sy,add_ex,add_ey,add_x, add_y,Line_Angle;
	double sect_sx, sect_sy, sect_ex, sect_ey;

	while(ccnt!=100)
	{
		if(ccnt!=200)
		{
			int nOriginLayer;

			if(nLayerNum >= m_iLayerCount)
				nOriginLayer = m_iLayerCount -1;
			else
				nOriginLayer = nLayerNum;

			sx = m_pLayer[nOriginLayer].pOriginEntity[nEntityNum].tempLine[nTemp].sX;
			sy = m_pLayer[nOriginLayer].pOriginEntity[nEntityNum].tempLine[nTemp].sY;
			ex = m_pLayer[nOriginLayer].pOriginEntity[nEntityNum].tempLine[nTemp].eX;
			ey = m_pLayer[nOriginLayer].pOriginEntity[nEntityNum].tempLine[nTemp].eY;
		}

		sect = 0;
		ccnt = 0;
		
		for( i=0; i<m_iMaxBlockNumber; i++)
		{
			if(m_b3StepMode && nLayerNum >= m_iLayerCount)
			{
				double dShift;

				if(nLayerNum == m_iLayerCount)
					dShift = 0.2;
				else if(nLayerNum == m_iLayerCount +1)
					dShift = -0.2;

				//첫 X열
				if( i % m_iBlockCountX == 0)					 sect_sx = m_pBlockPos[i].fLeft;
				else										 sect_sx = m_pBlockPos[i].fLeft	+ dShift;				

				//첫 Y열
				if( i < m_iMaxBlockNumber / m_iBlockCountY ) sect_sy = m_pBlockPos[i].fBottom;
				else										 sect_sy = m_pBlockPos[i].fBottom + dShift;

				//마지막 X열
				if( i % m_iBlockCountX == m_iBlockCountX-1 ) sect_ex = m_pBlockPos[i].fRight;
				else						 			     sect_ex = m_pBlockPos[i].fRight + dShift;

				//마지막 Y열
				if( i / m_iBlockCountX >= m_iBlockCountY-1 ) sect_ey = m_pBlockPos[i].fTop;
				else										 sect_ey = m_pBlockPos[i].fTop + dShift;
			}
			else
			{
				sect_sx = m_pBlockPos[i].fLeft;
				sect_sy = m_pBlockPos[i].fBottom;
				sect_ex = m_pBlockPos[i].fRight;
				sect_ey = m_pBlockPos[i].fTop;
			}

			if( ( (Change_um(sx)>=Change_um(sect_sx)) && (Change_um(sx)<=Change_um(sect_ex)) ) &&
				( (Change_um(ex)>=Change_um(sect_sx)) && (Change_um(ex)<=Change_um(sect_ex)) ) &&
				( (Change_um(sy)>=Change_um(sect_sy)) && (Change_um(sy)<=Change_um(sect_ey)) ) &&
				( (Change_um(ey)>=Change_um(sect_sy)) && (Change_um(ey)<=Change_um(sect_ey)) ) )
			{
				ccnt = 100;
				dummy = 0;
				nLineCnt++;
				SaveLineData(i ,sx,sy,ex,ey);
				break;
			}
			else   // sector를 벗어남
			{
				if( ((Change_um(sx)>=Change_um(sect_sx)) && (Change_um(sy)>=Change_um(sect_sy))) &&
					((Change_um(sx)<=Change_um(sect_ex)) && (Change_um(sy)<=Change_um(sect_ey))) )
				{
					dummy = 1; // 시작점이 Sector안에 있을경우 
					ret = SCAN_FindIntersection( (double)sx,(double)sy,(double)ex,(double)ey,
						(double)(sect_sx),(double)(sect_ex),(double)(sect_sy),(double)(sect_ey),
						&x,&y, dummy);
					if(ret)
					{
						nLineCnt++;
						//sect 를 나눈 선분에 길이 양쪽으로 증가
						Line_Angle = CalculateAngle2Point(sx,sy,x,y);
						Line_Angle=Line_Angle/180*PHI;
						add_sx=sx;//-0.1*cos(Line_Angle);
						add_sy=sy;//-0.1*sin(Line_Angle);
						add_x=x;
						add_y=y;
						add_x=x+m_fOverlap*cos(Line_Angle);
						add_y=y+m_fOverlap*sin(Line_Angle);
						SaveLineData(i,add_sx,add_sy,add_x,add_y);
						sx = x;
						sy = y;
						if(ret==1) ccnt=200;
					}
				}
				else if( ((Change_um(ex)>=Change_um(sect_sx)) && (Change_um(ey)>=Change_um(sect_sy))) &&
					((Change_um(ex)<=Change_um(sect_ex)) && (Change_um(ey)<=Change_um(sect_ey))) )
				{
					dummy = 2; // 끝점이 Sector안에 있을경우
					ret = SCAN_FindIntersection( (double)sx,(double)sy,(double)ex,(double)ey,
						(double)(sect_sx),(double)(sect_ex),(double)(sect_sy),(double)(sect_ey),
						&x,&y, dummy);
					if(ret)
					{
						nLineCnt++;
						Line_Angle = CalculateAngle2Point(x,y,ex,ey);

						if(Line_Angle > 89.999999)
							Line_Angle = Line_Angle;

						if(Line_Angle >89.999999 && Line_Angle < 90.000001)
							Line_Angle = 270;

						Line_Angle=Line_Angle/180*PHI;
						add_x=x;
						add_y=y;
						add_x=x-m_fOverlap*cos(Line_Angle);
						add_y=y-m_fOverlap*sin(Line_Angle);
						add_ex=ex;//+0.1*cos(Line_Angle);
						add_ey=ey;//+0.1*sin(Line_Angle);
						SaveLineData(i,add_x,add_y,add_ex,add_ey);
						ex = x;
						ey = y;
						if(ret==1) ccnt=200;
					}
				}
			}
			if(ccnt==200)
			{
				break;
			}
			if(ccnt==100)
				break;
		}
		if(ccnt == 0)
			break;
	}
	return nLineCnt;
}


int CFileEntity::SCAN_FindIntersection(double sx,double sy,double ex,double ey, double minx, double maxx, 
	double miny, double maxy, double *x, double *y, int dummy)
{
	double m,b;
	double endx, endy, startx, starty;

	*x = *y = 0;
	if(ex==sx)		// Vertical line
	{
		*x = sx;
		if(dummy==1)
		{
			if(ey<=miny) *y = miny;		// 아래방향
			else if(ey>=maxy) *y = maxy;// 윗방향
		}
		if(dummy==2)
		{
			if(sy<=miny) *y = miny;
			else if(sy>=maxy) *y = maxy;
		}
		return 2;
	}
	else if(ey==sy)		// Horizontal line
	{
		*y = sy;
		if(dummy==1)
		{
			if(ex<=minx) *x = minx;		// 왼쪽방향
			else if(ex>=maxx) *x = maxx;// 오른쪽방향
		}
		if(dummy==2)
		{
			if(sx<=minx) *x = minx;
			else if(sx>=maxx) *x = maxx;
		}
		return 2;
	}
	else
	{
		endx = ex;
		startx = sx;
		endy = ey;
		starty = sy;
		if(sx>ex)
		{
			endx = sx;
			startx = ex;
		}
		if(sy>ey)
		{
			endy = sy;
			starty = ey;
		}

		m = (ey-sy)/(ex-sx);
		b = ey-m*ex;

		// y = miny와의 교점
		*x = ((miny-b)/m);
		*y = (miny); 
		if( (*x>=minx && *x<=maxx) && (*y>=miny && *y<=maxy) )
		{
			if( (*x>startx && *x<endx) && (*y>starty && *y<endy) )
				return 1;
		}

		*x = ((maxy-b)/m);
		*y = (maxy); 
		if( (*x>startx && *x<endx) && (*y>starty && *y<endy) )
		{
			if( (*x>=minx && *x<=maxx) && (*y>=miny && *y<=maxy) )
				return 1;
		}

		// x = minx와의 교점
		*x = (minx);
		*y = (m*minx)+b;
		if( (*x>startx && *x<endx) && (*y>starty && *y<endy) )
		{
			if( (*x>=minx && *x<=maxx) && (*y>=miny && *y<=maxy) )
				return 1;
		}

		// x = maxx와의 교점
		*x = (maxx);
		*y = (m*maxx+b); 
		if( (*x>startx && *x<endx) && (*y>starty && *y<endy) )
		{
			if( (*x>=minx && *x<=maxx) && (*y>=miny && *y<=maxy) )
				return 1;
		}

		return 0;
	}

	return 1;
}


double CFileEntity::CalculateAngle2Point(double x1, double y1, double x2, double y2)
{
	double ang;
	if((x2 != x1) || (y2 != y1))
	{
		double tan_ang = atan((y2 - y1)/(x2 - x1));
		ang = tan_ang * (180.0 / PHI);

		if(((y2-y1)/(x2-x1))>0)
		{
			if(x2<x1)
				ang += 180;

		}
		else
		{
			if(x2>x1)
				ang = 360 + ang;
			else
				ang = 180 + ang;
		}

	}
	else if((x2!=x1) && (y2==y1))
	{
		if(x2>x1)
			ang = 0;
		else
			ang = 180;
	}
	else if(x2 == x1)
	{
		if(y2>y1)
			ang = 90;
		else
			ang = 270;
	}

	return ang;
}

int CFileEntity::SetBufferOriginEntity()
{
	/* Init Memory */




	/* Swap */

	for(int i=0; i<m_iLayerCount; i++)
	{
		m_pLayer[i].pOriginEntity = new Entity_Information[m_pLayer[i].iEntityCount];

		m_pLayer[i].iOriginCount = m_pLayer[i].iEntityCount;

		for(int j=0; j<m_pLayer[i].iEntityCount; j++)
		{
			m_pLayer[i].pOriginEntity[j] = m_pLayer[i].pEntity[j];
		}
	}
	return 1;
}


int CFileEntity::ApplyUserSector(void)
{
	SetTotalProgress();

	before = clock();

	int i;			/** i local variable 01*/

	for(int i=0; i<m_nLayerBufferSize; i++)
	{
		if(m_pLayer[i].pCutBlock != NULL)
		{
			for(int j=0;j<m_iMaxBlockNumber;j++)
			{
				if(m_pLayer[i].pCutBlock[j].m_pEntity != NULL){
					delete [] m_pLayer[i].pCutBlock[j].m_pEntity;
					m_pLayer[i].pCutBlock[j].m_pEntity= NULL;
				}
			}
			delete [] m_pLayer[i].pCutBlock;
			m_pLayer[i].pCutBlock = NULL;
		}
	}

	MakeBlockAllLayer();

	for(i=0;i<m_nLayerBufferSize; i++)
	{			
		if(!MakeBlock(i))				
			return 0;

		/* 1. Read data from Block Point file.
		2. Sort block data by block index.
		3. Save file */
		if(!MakeBlockSort(i) || m_bLoadStopFlag == TRUE)			
			return 0;

		/* Sort Block by stage path optimization */
		if(!SortBlock_StagePos(i) || m_bLoadStopFlag == TRUE)
			return 0;

		/* Make Block Information file */
		if(!SaveBlockInformation(i) || m_bLoadStopFlag == TRUE)		
			return 0;

		//Cutting Buffer

		/* Load Block data from sorted block data file */
		if(!LoadBlockData(i) || m_bLoadStopFlag == TRUE)		
			return 0;

		/* Optimization option */
 		if(m_bOptimization == TRUE)	
 		{
 			if(!DoOptimization(i))
 			{
 				return 0;
 			}
 		}
	}

	/* Optimization option */
	//if(m_bOptimization == TRUE)	
	//{
	//	StartOptimize();
	//}

	//작업 번호에 따라 포인트를 정리함

	//AfxMessageBox(strTime);

	return 1;
}

void CFileEntity::InitSectorOrder(int iLayerNum)
{
	int cnt=0;			/** cnt local variable 05*/
	int i;

	for(i = 0;i < m_iMaxBlockNumber; i++)
	{
		m_iProgressCount++;
		
		if(m_bLoadStopFlag == TRUE)
			return;

		if( m_pLayer[iLayerNum].pInBlockEntityCount[i] > 0)
		{
			m_pLayer[iLayerNum].pCutBlock[i].lOrderNumber = cnt;
			m_pLayer[iLayerNum].pBlockOrder[cnt] = i;
			cnt++;
		}
		else
		{
			m_pLayer[iLayerNum].pCutBlock[i].lOrderNumber = _SECT_EMPTY;
		}
	}
	m_pLayer[iLayerNum].iRealBlockCount = cnt;
}

void CFileEntity::InputSectorOrder()
{
	int cnt=0;			/** cnt local variable 05*/
	int i;

	for(i = 0;i < m_iMaxBlockNumber; i++)
	{
		m_iProgressCount++;

		if(m_bLoadStopFlag == TRUE)
			return;
		
		if( m_pLayer[m_nSelLayer].pInBlockEntityCount[i] > 0)
		{
			m_pLayer[m_nSelLayer].pCutBlock[i].lOrderNumber = _SECT_INPUT;
			m_pLayer[m_nSelLayer].pBlockOrder[cnt] = _SECT_INPUT;
			cnt++;
		}
		else
		{
			m_pLayer[m_nSelLayer].pCutBlock[i].lOrderNumber = _SECT_EMPTY;
		}
	}
	m_pLayer[m_nSelLayer].iRealBlockCount = cnt;
}

void CFileEntity::SelectLayer(int iLayerNumber)
{
	m_nSelLayer = iLayerNumber;
}

void CFileEntity::ModifyORGCoord(double fMoveX, double fMoveY)
{
	//Min Max Coord
	m_fTotalMinX -= fMoveX;
	m_fTotalMinY -= fMoveY;
	m_fTotalMaxX -= fMoveX;
	m_fTotalMaxY -= fMoveY;

	//Entity Coord
	for(int nLayerCnt = 0; nLayerCnt<m_nLayerBufferSize; nLayerCnt++)
	{
		for(int nEntityCnt = 0; nEntityCnt<m_pLayer[nLayerCnt].iOriginCount; nEntityCnt++)
		{
			m_pLayer[nLayerCnt].pOriginEntity[nEntityCnt].cX -= fMoveX;
			m_pLayer[nLayerCnt].pOriginEntity[nEntityCnt].cY -= fMoveY;

			m_pLayer[nLayerCnt].pOriginEntity[nEntityCnt].sX -= fMoveX;
			m_pLayer[nLayerCnt].pOriginEntity[nEntityCnt].sY -= fMoveY;

			m_pLayer[nLayerCnt].pOriginEntity[nEntityCnt].eX -= fMoveX;
			m_pLayer[nLayerCnt].pOriginEntity[nEntityCnt].eY -= fMoveY;
		}
	}

	//Sector Coord
	for(int nRowCnt = 0; nRowCnt<m_SectLine.RowCnt; nRowCnt++)
	{
		m_SectLine.Row[nRowCnt].sx -= Change_um(fMoveX);
		m_SectLine.Row[nRowCnt].ex -= Change_um(fMoveX);
	}

	for(int nColCnt = 0; nColCnt<m_SectLine.ColCnt; nColCnt++)
	{
		m_SectLine.Col[nColCnt].sy -= Change_um(fMoveY);
		m_SectLine.Col[nColCnt].ey -= Change_um(fMoveY);
	}
}

void CFileEntity::ArcToLine(int nLayer, int nEntity)
{
	double sx, sy, ex, ey, ang, temp_a,temp_b,temp_sa,temp_ea, r, cx, cy;
	int i=0;
	double add_i;

	m_pLayer[nLayer].pOriginEntity[nEntity].ang = 
		m_pLayer[nLayer].pOriginEntity[nEntity].eA - m_pLayer[nLayer].pOriginEntity[nEntity].sA;

	temp_sa = m_pLayer[nLayer].pOriginEntity[nEntity].sA;
	temp_ea = m_pLayer[nLayer].pOriginEntity[nEntity].eA;

	r = m_pLayer[nLayer].pOriginEntity[nEntity].rad;

	cx = m_pLayer[nLayer].pOriginEntity[nEntity].cX;
	cy = m_pLayer[nLayer].pOriginEntity[nEntity].cY;
	
	ang = (temp_sa) * PHI / 180.;
	temp_a = cx + (r * cos(ang));
	temp_b = cy + (r * sin(ang));

	sx = temp_a;
	sy = temp_b;

	if( temp_sa > temp_ea) temp_ea += 360;

	if(r<=0.05)
	{
		add_i = 30;
	}
	else if(r<0.1)
	{
		add_i = 20;
	}
	else if(r<200)
	{
		add_i = 10;
	}
	else if(r<500)
	{
		add_i = 1;
	}
	else if(r<1000)
	{
		add_i = 0.5;
	}
	else if(r<2000)
	{
		add_i = 0.11;
	}
	else
	{
		add_i = 0.01; //add_i 는 각도
	}

	int nCnt = 0;
	double dCopySa = temp_sa;
	for(;;)
	{
		dCopySa += add_i;
		nCnt++;

		if(DoRound(dCopySa)>=DoRound(temp_ea)) 
		{
			break;
		}
	}

	m_pLayer[nLayer].pOriginEntity[nEntity].tempSize = nCnt;
	m_pLayer[nLayer].pOriginEntity[nEntity].tempLine = new Entity_Information[nCnt];
	
	if(m_fTotalMinX > sx) m_fTotalMinX = sx;
	if(m_fTotalMaxX < sx) m_fTotalMaxX = sx;
	if(m_fTotalMinY > sy) m_fTotalMinY = sy;
	if(m_fTotalMaxY < sy) m_fTotalMaxY = sy;

	nCnt = 0;
	for(;;)
	{
		temp_sa += add_i;
		
		if(DoRound(temp_sa)>=DoRound(temp_ea)) 
		{
			temp_sa = temp_ea;
			i = 100;
		}

		ang = (temp_sa) * PHI / 180.;
		temp_a = cx + (r * cos(ang));
		temp_b = cy + (r * sin(ang));

		ex = temp_a;
		ey = temp_b;

		m_pLayer[nLayer].pOriginEntity[nEntity].tempLine[nCnt].nType = _ENTITY_LINE;
		m_pLayer[nLayer].pOriginEntity[nEntity].tempLine[nCnt].sX = sx;
		m_pLayer[nLayer].pOriginEntity[nEntity].tempLine[nCnt].eX = ex;
		m_pLayer[nLayer].pOriginEntity[nEntity].tempLine[nCnt].sY = sy;
		m_pLayer[nLayer].pOriginEntity[nEntity].tempLine[nCnt].eY = ey;
		nCnt++;

		if(m_fTotalMinX > ex) m_fTotalMinX = ex;
		if(m_fTotalMaxX < ex) m_fTotalMaxX = ex;
		if(m_fTotalMinY > ey) m_fTotalMinY = ey;
		if(m_fTotalMaxY < ey) m_fTotalMaxY = ey;

		sx = ex;
		sy = ey;

		if(i==100) break;
	}
}

void CFileEntity::EllipseToLine(int nLayer, int nEntity)
{
	double sx, sy, ex, ey, radAng, temp_x,temp_y,temp_sa,temp_ea, ratio, cx, cy, radRotation;
	int i=0;
	double add_i;
	double A, B;

	temp_sa = m_pLayer[nLayer].pOriginEntity[nEntity].sA;
	temp_ea = m_pLayer[nLayer].pOriginEntity[nEntity].eA;

	ratio = m_pLayer[nLayer].pOriginEntity[nEntity].rad;

	sx = m_pLayer[nLayer].pOriginEntity[nEntity].sX;
	sy = m_pLayer[nLayer].pOriginEntity[nEntity].sY;

	cx = m_pLayer[nLayer].pOriginEntity[nEntity].cX;
	cy = m_pLayer[nLayer].pOriginEntity[nEntity].cY;

	A = Calculate2PointDistance(sx, sy, cx, cy); //장축
	B = A*ratio;								//단축

	radRotation = atan2(sy - cy, sx - cx);// * 180. / PHI;
		
	radAng = temp_sa * PHI / 180.;
	temp_x = cx + A * cos(radAng)*cos(radRotation) - B * sin(radAng) * sin(radRotation);
	temp_y = cy + A * cos(radAng)*sin(radRotation) + B * sin(radAng) * cos(radRotation);

	sx = temp_x;
	sy = temp_y;

	if( temp_sa > temp_ea) temp_ea += 360;

	if(A<=0.05)
	{
		add_i = 30;
	}
	else if(A<0.1)
	{
		add_i = 20;
	}
	else if(A<200)
	{
		add_i = 10;
	}
	else if(A<500)
	{
		add_i = 1;
	}
	else if(A<1000)
	{
		add_i = 0.5;
	}
	else if(A<2000)
	{
		add_i = 0.11;
	}
	else
	{
		add_i = 0.01; //add_i 는 각도
	}

	int nCnt = 0;
	double dCopySa = temp_sa;
	for(;;)
	{
		dCopySa += add_i;
		nCnt++;

		if(DoRound(dCopySa)>=DoRound(temp_ea)) 
		{
			break;
		}
	}

	m_pLayer[nLayer].pOriginEntity[nEntity].tempSize = nCnt;
	m_pLayer[nLayer].pOriginEntity[nEntity].tempLine = new Entity_Information[nCnt];

	if(m_fTotalMinX > sx) m_fTotalMinX = sx;
	if(m_fTotalMaxX < sx) m_fTotalMaxX = sx;
	if(m_fTotalMinY > sy) m_fTotalMinY = sy;
	if(m_fTotalMaxY < sy) m_fTotalMaxY = sy;

	nCnt = 0;
	for(;;)
	{
		temp_sa += add_i;

		if(DoRound(temp_sa)>=DoRound(temp_ea)) 
		{
			temp_sa = temp_ea;
			i = 100;
		}

		radAng = (temp_sa) * PHI / 180.;

		temp_x = cx + A * cos(radAng)*cos(radRotation) - B * sin(radAng) * sin(radRotation);
		temp_y = cy + A * cos(radAng)*sin(radRotation) + B * sin(radAng) * cos(radRotation);

		ex = temp_x;
		ey = temp_y;

		m_pLayer[nLayer].pOriginEntity[nEntity].tempLine[nCnt].nType = _ENTITY_LINE;
		m_pLayer[nLayer].pOriginEntity[nEntity].tempLine[nCnt].sX = sx;
		m_pLayer[nLayer].pOriginEntity[nEntity].tempLine[nCnt].eX = ex;
		m_pLayer[nLayer].pOriginEntity[nEntity].tempLine[nCnt].sY = sy;
		m_pLayer[nLayer].pOriginEntity[nEntity].tempLine[nCnt].eY = ey;
		nCnt++;

		if(m_fTotalMinX > ex) m_fTotalMinX = ex;
		if(m_fTotalMaxX < ex) m_fTotalMaxX = ex;
		if(m_fTotalMinY > ey) m_fTotalMinY = ey;
		if(m_fTotalMaxY < ey) m_fTotalMaxY = ey;

		sx = ex;
		sy = ey;

		if(i==100) break;
	}
}

void CFileEntity::AllocLayerMem(int nSize)
{
	m_pLayer = new LAYER[nSize];

	for(int i=0; i<nSize; i++)
	{
		m_pLayer[i].pOriginEntity = NULL;
		m_pLayer[i].pInBlockEntityCount = NULL;
		m_pLayer[i].pBlockOrder = NULL;
		m_pLayer[i].pCutBlock = NULL;
		m_pLayer[i].pEntity = NULL;
	}
}

void CFileEntity::AllocLayerEntityMem(int nLayer, int nSize)
{
	m_pLayer[nLayer].pOriginEntity = new Entity_Information[nSize];

	for(int i=0; i<nSize; i++)
	{
		m_pLayer[nLayer].pOriginEntity[i].tempLine = NULL;
	}
}

void CFileEntity::AllocSectLineMem(int nRowSize, int nColSize)
{
	m_SectLine.Row = new LINE[nRowSize];
	m_SectLine.Col = new LINE[nColSize];
}

void CFileEntity::AllocCutBlock(int nLayer, int nSize)
{
	m_pLayer[nLayer].pCutBlock = new BLOCK_INFO[nSize];
	m_pLayer[nLayer].pInBlockEntityCount = new int[nSize];
	m_pLayer[nLayer].pBlockOrder = new int[nSize];

	for(int i=0; i<nSize; i++){
		m_pLayer[nLayer].pCutBlock[i].m_pEntity = NULL;
		m_pLayer[nLayer].pInBlockEntityCount[i] = 0;
		m_pLayer[nLayer].pBlockOrder[i]	= -1;
	}
}

void CFileEntity::AllocCutBlockEntity(int nLayer, int nBlock, int nSize)
{
	m_pLayer[nLayer].pCutBlock[nBlock].m_pEntity = new Entity_Information[nSize];
	
	m_pLayer[nLayer].pInBlockEntityCount[nBlock] = nSize;

	for(int i=0; i<nSize; i++){
		m_pLayer[nLayer].pCutBlock[nBlock].m_pEntity[i].tempLine = NULL;
	}
}


void CFileEntity::SetCenterAlign(BOOL bFlag)
{
	m_bCenterAlign = bFlag;
}

void CFileEntity::SetRemoveBlank(BOOL bFlag)
{
	m_bRemoveBlank = bFlag;
}

int CFileEntity::ExtensionSectorLine(int LineType, BOOL bFlag)
{
	if(bFlag)
	{
		if(LineType == _ROW_LINE)
		{
			LINE *tempLine = new LINE[m_SectLine.RowCnt];

			for(int i=0; i< m_SectLine.RowCnt; i++)
			{
				tempLine[i] = m_SectLine.Row[i];
			}

			delete[] m_SectLine.Row;

			int nNewCount = m_SectLine.RowCnt+1;

			m_SectLine.Row = new LINE[nNewCount];

			for(int i=0; i<m_SectLine.RowCnt; i++)
			{
				m_SectLine.Row[i].sx = tempLine[i].sx;
				m_SectLine.Row[i].ex = tempLine[i].ex;
				m_SectLine.Row[i].sy = tempLine[i].sy;
				m_SectLine.Row[i].ey = tempLine[i].ey;
			}
			m_SectLine.Row[nNewCount-1].sx = m_SectLine.Row[nNewCount-2].sx + m_iBlockSize;
			m_SectLine.Row[nNewCount-1].ex = m_SectLine.Row[nNewCount-2].ex	+ m_iBlockSize;
			m_SectLine.Row[nNewCount-1].sy = m_SectLine.Row[nNewCount-2].sy;
			m_SectLine.Row[nNewCount-1].ey = m_SectLine.Row[nNewCount-2].ey;

			m_SectLine.RowCnt = nNewCount;

			delete[] tempLine;
		}
		else if(LineType == _COL_LINE)
		{
			LINE *tempLine = new LINE[m_SectLine.ColCnt];

			for(int i=0; i< m_SectLine.ColCnt; i++)
			{
				tempLine[i] = m_SectLine.Col[i];
			}

			delete[] m_SectLine.Col;

			int nNewCount = m_SectLine.ColCnt + 1;

			m_SectLine.Col = new LINE[nNewCount];

			for(int i=0; i<m_SectLine.ColCnt; i++)
			{
				m_SectLine.Col[i].sx = tempLine[i].sx;
				m_SectLine.Col[i].ex = tempLine[i].ex;
				m_SectLine.Col[i].sy = tempLine[i].sy;
				m_SectLine.Col[i].ey = tempLine[i].ey;
			}
			m_SectLine.Col[nNewCount-1].sx = m_SectLine.Col[nNewCount-2].sx;
			m_SectLine.Col[nNewCount-1].ex = m_SectLine.Col[nNewCount-2].ex;
			m_SectLine.Col[nNewCount-1].sy = m_SectLine.Col[nNewCount-2].sy	+ m_iBlockSize;
			m_SectLine.Col[nNewCount-1].ey = m_SectLine.Col[nNewCount-2].ey	+ m_iBlockSize;

			m_SectLine.ColCnt = nNewCount;

			delete[] tempLine;
		}
	}
	else
	{
		if(LineType == _ROW_LINE)
		{
			if( m_SectLine.RowCnt < 3 ){
				AfxMessageBox(L" A Last Sector" );
				return 0;
			}

			LINE *tempLine = new LINE[m_SectLine.RowCnt];

			for(int i=0; i< m_SectLine.RowCnt; i++)
			{
				tempLine[i] = m_SectLine.Row[i];
			}

			delete[] m_SectLine.Row;

			int nNewCount = m_SectLine.RowCnt-1;

			m_SectLine.Row = new LINE[nNewCount];

			for(int i=0; i<nNewCount; i++)
			{
				m_SectLine.Row[i].sx = tempLine[i].sx;
				m_SectLine.Row[i].ex = tempLine[i].ex;
				m_SectLine.Row[i].sy = tempLine[i].sy;
				m_SectLine.Row[i].ey = tempLine[i].ey;
			}

			for(int i=0; i<m_SectLine.ColCnt; i++)
			{
				m_SectLine.Col[i].ex -= m_iBlockSize;
			}
			

			m_SectLine.RowCnt = nNewCount;

			delete[] tempLine;
		}
		else if(LineType == _COL_LINE)
		{
			if( m_SectLine.ColCnt < 3 ){
				AfxMessageBox(L" A Last Sector" );
				return 0;
			}

			LINE *tempLine = new LINE[m_SectLine.ColCnt];

			for(int i=0; i< m_SectLine.ColCnt; i++)
			{
				tempLine[i] = m_SectLine.Col[i];
			}

			delete[] m_SectLine.Col;

			int nNewCount = m_SectLine.ColCnt - 1;

			m_SectLine.Col = new LINE[nNewCount];

			for(int i=0; i<nNewCount; i++)
			{
				m_SectLine.Col[i].sx = tempLine[i].sx;
				m_SectLine.Col[i].ex = tempLine[i].ex;
				m_SectLine.Col[i].sy = tempLine[i].sy;
				m_SectLine.Col[i].ey = tempLine[i].ey;
			}

			for(int i=0; i<m_SectLine.RowCnt; i++)
			{
				m_SectLine.Row[i].ey -= m_iBlockSize;
			}
			
			m_SectLine.ColCnt = nNewCount;

			delete[] tempLine;
		}
	}

	return 1;
}


BOOL CFileEntity::CheckSectorArea(void)
{
	double dAreaMinX, dAreaMinY, dAreaMaxX, dAreaMaxY;
	
	dAreaMinX = m_SectLine.Row[0].sx;
	dAreaMaxX = m_SectLine.Row[m_SectLine.RowCnt-1].ex;

	dAreaMinY = m_SectLine.Col[0].sy;
	dAreaMaxY = m_SectLine.Col[m_SectLine.ColCnt-1].ey;

	if( (dAreaMinX > m_fTotalMinX ) ||
		(dAreaMinY > m_fTotalMinY )	||
		(dAreaMaxX < m_fTotalMaxX )	||
		(dAreaMaxY < m_fTotalMaxY ) ){
		return FALSE;
	}
	else
		return TRUE;
}

void CFileEntity::SetSectorCenterAlign(void)
{
	double dAreaMinX, dAreaMinY, dAreaMaxX, dAreaMaxY;

	dAreaMinX = m_SectLine.Row[0].sx;
	dAreaMaxX = m_SectLine.Row[m_SectLine.RowCnt-1].ex;

	dAreaMinY = m_SectLine.Col[0].sy;
	dAreaMaxY = m_SectLine.Col[m_SectLine.ColCnt-1].ey;

	double dAreaCx = ( dAreaMinX + dAreaMaxX ) / 2;
	double dAreaCy = ( dAreaMinY + dAreaMaxY ) / 2;

	double dDrawCx = ( m_fTotalMinX + m_fTotalMaxX) / 2;
	double dDrawCy = ( m_fTotalMinY + m_fTotalMaxY) / 2;

	double dMoveX = dDrawCx - dAreaCx;
	double dMoveY = dDrawCy - dAreaCy;

	m_dSectorTotalMoveX = dMoveX + m_dSectorTotalMoveX;
	m_dSectorTotalMoveY = dMoveY + m_dSectorTotalMoveY;

	for(int i=0; i<m_SectLine.RowCnt; i++)
	{
		m_SectLine.Row[i].sx += dMoveX;
		m_SectLine.Row[i].ex += dMoveX;
		m_SectLine.Row[i].sy += dMoveY;
		m_SectLine.Row[i].ey += dMoveY;
	}

	for(int i=0; i<m_SectLine.ColCnt; i++)
	{
		m_SectLine.Col[i].sx += dMoveX;
		m_SectLine.Col[i].ex += dMoveX;
		m_SectLine.Col[i].sy += dMoveY;
		m_SectLine.Col[i].ey += dMoveY;
	}
}

void CFileEntity::GetSectorTotalMove(double *dX, double *dY)
{
	*dX = m_dSectorTotalMoveX;
	*dY = m_dSectorTotalMoveY;
}

BOOL CFileEntity::StartOptimize(void)
{
	CWinThread **THTH = new CWinThread*[m_nLayerBufferSize];
	
	for(int i=0; i<m_nLayerBufferSize; i++)
	{
		Sleep(100);
		nNum = i;
		THTH[i] = ::AfxBeginThread(ThreadOptimize, this, THREAD_PRIORITY_NORMAL,0,CREATE_SUSPENDED);
		THTH[i]->ResumeThread();	
		
	}

	for( int i=0; i<m_nLayerBufferSize; i++)
		WaitForSingleObject(THTH[i]->m_hThread, INFINITE);
	
	return FALSE;
}


UINT ThreadOptimize(LPVOID pParam)
{
	CFileEntity* pData = (CFileEntity*)pParam;
	
	int nNum = pData->nNum;

	pData->DoOptimization(nNum);
	
	return 0;
}


void CFileEntity::ApplyInspectResult()
{
	double *idealX = m_fIdealPointX;
	double *idealY = m_fIdealPointY;
	double *realX =  m_fRealPointX;
	double *realY =  m_fRealPointY;

	// A[] * H[] = B[] -> H[] = invA[] * B[]
	double Array_A[8 * 8] = {
		idealX[0], idealY[0], 1, 0, 0, 0, (-1) * idealX[0] * realX[0], (-1) * realX[0] * idealY[0],
		0, 0, 0, idealX[0], idealY[0], 1, (-1) * idealX[0] * realY[0], (-1) * idealY[0] * realY[0], 

		idealX[1], idealY[1], 1, 0, 0, 0, (-1) * idealX[1] * realX[1], (-1) * realX[1] * idealY[1],
		0, 0, 0, idealX[1], idealY[1], 1, (-1) * idealX[1] * realY[1], (-1) * idealY[1] * realY[1], 

		idealX[2], idealY[2], 1, 0, 0, 0, (-1) * idealX[2] * realX[2], (-1) * realX[2] * idealY[2],
		0, 0, 0, idealX[2], idealY[2], 1, (-1) * idealX[2] * realY[2], (-1) * idealY[2] * realY[2], 

		idealX[3], idealY[3], 1, 0, 0, 0, (-1) * idealX[3] * realX[3], (-1) * realX[3] * idealY[3],
		0, 0, 0, idealX[3], idealY[3], 1, (-1) * idealX[3] * realY[3], (-1) * idealY[3] * realY[3] 
	};

	double Array_H[8*1] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	double Array_B[8*1] = { realX[0], realY[0], realX[1], realY[1], realX[2], realY[2], realX[3], realY[3] };


	CMatrix mtrx_A(Array_A, 8, 8);

	CMatrix mtrx_Inv_A = mtrx_A.inverse();

	CMatrix mtrx_H(Array_H, 8, 1);

	CMatrix mtrx_B(Array_B, 8, 1);

	mtrx_H = mtrx_Inv_A * mtrx_B;

	for(int nIndex=0; nIndex < 8; nIndex++)
	{
		dHomography[ nIndex ] = mtrx_H(nIndex, 0);
	}

	double dSX, dSY, dEX, dEY, dCX, dCY;

	double* dHomo = dHomography;

	for(int nLayer = 0; nLayer < m_iLayerCount; nLayer++)
	{
		for(int i=0; i<m_pLayer[nLayer].iOriginCount; i++)
		{
			//sx, sy : POINT, ARC, LINE, CIRCLE
			dSX = ( dHomo[0] * m_pLayer[nLayer].pOriginEntity[i].sX + dHomo[1] * m_pLayer[nLayer].pOriginEntity[i].sY +  dHomo[2] ) / 
				( dHomo[6] * m_pLayer[nLayer].pOriginEntity[i].sX + dHomo[7] * m_pLayer[nLayer].pOriginEntity[i].sY +  1 );

			dSY = ( dHomo[3] * m_pLayer[nLayer].pOriginEntity[i].sX + dHomo[4] * m_pLayer[nLayer].pOriginEntity[i].sY +  dHomo[5] ) / 
				( dHomo[6] * m_pLayer[nLayer].pOriginEntity[i].sX + dHomo[7] * m_pLayer[nLayer].pOriginEntity[i].sY +  1 );

			m_pLayer[nLayer].pOriginEntity[i].sX = dSX;
			m_pLayer[nLayer].pOriginEntity[i].sY = dSY;

			//ex, ey : LINE
			dEX = ( dHomo[0] * m_pLayer[nLayer].pOriginEntity[i].eX + dHomo[1] * m_pLayer[nLayer].pOriginEntity[i].eY +  dHomo[2] ) / 
				( dHomo[6] * m_pLayer[nLayer].pOriginEntity[i].eX + dHomo[7] * m_pLayer[nLayer].pOriginEntity[i].eY +  1 );

			dEY = ( dHomo[3] * m_pLayer[nLayer].pOriginEntity[i].eX + dHomo[4] * m_pLayer[nLayer].pOriginEntity[i].eY +  dHomo[5] ) / 
				( dHomo[6] * m_pLayer[nLayer].pOriginEntity[i].eX + dHomo[7] * m_pLayer[nLayer].pOriginEntity[i].eY +  1 );

			m_pLayer[nLayer].pOriginEntity[i].eX = dEX;
			m_pLayer[nLayer].pOriginEntity[i].eY = dEY;

			//Radius : CIRCLE, ARC
			if(m_pLayer[nLayer].pOriginEntity[i].nType == _ENTITY_CIRCLE || 
				m_pLayer[nLayer].pOriginEntity[i].nType == _ENTITY_ARC)
			{
				dCX = ( dHomo[0] * m_pLayer[nLayer].pOriginEntity[i].cX + dHomo[1] * m_pLayer[nLayer].pOriginEntity[i].cY +  dHomo[2] ) / 
					( dHomo[6] * m_pLayer[nLayer].pOriginEntity[i].cX + dHomo[7] * m_pLayer[nLayer].pOriginEntity[i].cY +  1 );

				dCY = ( dHomo[3] * m_pLayer[nLayer].pOriginEntity[i].cX + dHomo[4] * m_pLayer[nLayer].pOriginEntity[i].cY +  dHomo[5] ) / 
					( dHomo[6] * m_pLayer[nLayer].pOriginEntity[i].cX + dHomo[7] * m_pLayer[nLayer].pOriginEntity[i].cY +  1 );

				m_pLayer[nLayer].pOriginEntity[i].cX = dCX;
				m_pLayer[nLayer].pOriginEntity[i].cY = dCY;
			}	
		}
	}
}