#include "StdAfx.h"
#include "FileDxf.h"
#include <math.h>
#include <locale.h>

#define PHI 3.141592653589793238462643383279 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CFileDxf::CFileDxf(void)
	: MemoryBuf(_T(""))
{
	setlocale(LC_ALL,"Korean");

	m_pLineType = NULL;
	m_pLayer = NULL;
	m_pLayerName = NULL;
	m_iLineTYPEConut=0;
	m_iLayerCount=0;
	m_pFileCoord = NULL;
	m_pFileLoad = NULL;
	m_pFileWrite = NULL;
	bLoadColorFlag = false;
	m_bLoadStopFlag = false;
	m_iColorNumber = 0;
	m_iDxfVersion = 0;
	memset( &m_iLayerColor, 0x0, sizeof(m_iLayerColor) );
	memset( &m_iLayerPointCount, 0x0, sizeof(m_iLayerPointCount) );
	m_iMaxLayerNumber = 0;
	m_iProgressCount = 0;
	m_iTotalProgress = 0;
	m_lFileLength = 0;

	InitializeFileFolder();
}


CFileDxf::~CFileDxf(void)
{
	InitializeFileFolder();
}

bool CFileDxf::LoadDxfFile(CString i_strFileName)
{
	int	g_code;					/** g_code local variable 01*/
	bool continue_flag;			/** continue_flag local variable 02*/
	char path[1000];			/** path local variable 03*/
	wsprintfA(path,"%S",i_strFileName);

	/* Clear temporay memory buffer */
	DeleteDxfBuffer();

	m_iProgressCount++;

	InitializeDxfVariable();

	m_iProgressCount++;

	/* Open entity file handle to save */
	OpenEntityFileSave();

	m_iProgressCount++;

	/* Open Dxf file */
	if((m_pFileLoad =_fsopen(path, "r",_SH_DENYNO))==NULL)
	{
		CloseEntityFileSave();
		return false;
	}

	m_iProgressCount++;

	/* Find Head & Check Dxf version */
	if (FindDxfHeader() == false) 
	{
		fclose(m_pFileLoad);
		CloseEntityFileSave();
		return false;
	}

	m_iProgressCount++;

	/* Allocate temporary memory buffer */
	AllocDxfBuffer();

	m_iProgressCount++;

	/* Find Table * Save Layer informations */
	if (FindDxfTable() == false) 
	{ 
		DeleteDxfBuffer(); 
		CloseEntityFileSave();
		fclose(m_pFileLoad);
		return false; 
	}

	m_iProgressCount++;

	// Blocks or Entities SECTION
	if (FindDxfEntry(0, _T("SECTION")) == false) 
	{ 
		DeleteDxfBuffer(); 
		CloseEntityFileSave();
		fclose(m_pFileLoad);
		return false; 
	}

	m_iProgressCount++;

	if (FindDxfEntry(2, _T("ENTITIES")) == false) 
	{ 
		DeleteDxfBuffer(); 
		CloseEntityFileSave();
		fclose(m_pFileLoad);
		return false; 
	}

	m_iProgressCount++;

	continue_flag = true;
	bLoadColorFlag = true;
	m_bIsPolyLoaded = FALSE;

	m_iProgressCount++;

	/* Find & Save Point Data */
	while (continue_flag)
	{
		if(m_bLoadStopFlag == TRUE)
		{
			fclose(m_pFileLoad);
			CloseEntityFileSave();
			return false;
		}

		if(GetVersion()==6)
		{
			if(m_bIsPolyLoaded == FALSE)
			{
				g_code = GetLine(MemoryBuf);
			}
			m_bIsPolyLoaded = FALSE;

		}
		else
		{
			//if(bLoadColorFlag == true)
			//{
				g_code = GetLine(MemoryBuf);
			//}
		}


		switch (g_code)
		{
		case DXF_ERR : 
			{ 
				continue_flag = false; 
				break; 
			}
		case 0 :	// file section mark
			if (MemoryBuf == _T("EOF")) 		/* End of File. Exit loop */
			{ 
				continue_flag = false; 
			}
			else if (MemoryBuf == _T("ENDSEC")) 	/* Section End. Exit loop */
			{ 
				continue_flag = false; 
			}
			else if (MemoryBuf == _T("POINT"))	/* Position informations. Save Point data */
			{
				bLoadColorFlag=false;
				DrawPoint();
			}
			else if (MemoryBuf == _T("LINE"))		
			{
				bLoadColorFlag=false;
				DrawLine();
			}
			else if (MemoryBuf == _T("CIRCLE"))
			{
				bLoadColorFlag=false;
				DrawCircle();
			}
			else if (MemoryBuf == _T("ARC"))
			{
				bLoadColorFlag=false;
				DrawArc();
			}
			else if (MemoryBuf == _T("POLYLINE"))
			{
				bLoadColorFlag=false;
				DrawPolyLine();
			}
			else if (MemoryBuf == _T("LWPOLYLINE"))
			{
				bLoadColorFlag=false;
				DrawPolyLine2000();
			}
			else if (MemoryBuf == _T("ELLIPSE"))
			{
				bLoadColorFlag=false;
				DrawEllipse();
			}
// 			else if (MemoryBuf == _T("TEXT"));
// 			else if (MemoryBuf == _T("SOLID"));
// 			else if (MemoryBuf == _T("INSERT"));
			break;
		}
	}

	m_iProgressCount++;

	if(SortLayerNumber()==0)
	{
		fclose(m_pFileLoad);
		CloseEntityFileSave();
		return false;
	}

	m_iProgressCount++;

	

	/* Close Dxf file handle */
	fclose(m_pFileLoad);

	/* Close entity file */
	CloseEntityFileSave();

	m_iProgressCount++;

	SortByLayerName();

	/* Make Common Entity file  */
	if(!MakeCommonEntityFile())
	{
		/* Clear temporay memory buffer */
		DeleteDxfBuffer();
		return false;
	}

	m_iProgressCount++;

	/* Clear temporay memory buffer */
	DeleteDxfBuffer();

	m_iProgressCount++;

	return true;
}

void CFileDxf::DeleteDxfBuffer(void)
{
	if (m_pLineType != NULL) 
	{ 
		delete [] m_pLineType; 
		m_pLineType = NULL; 
	}
	if (m_pLayer != NULL) 
	{ 
		delete [] m_pLayer; 
		m_pLayer = NULL; 
	}
	if (m_pLayerName != NULL) 
	{ 
		delete[] m_pLayerName; 
		m_pLayerName = NULL; 
	}

	m_iMaxLayerNumber = m_iLayerCount;

	m_iLineTYPEConut=0;
}


void CFileDxf::InitializeDxfVariable(void)
{
	m_nInspectionCnt = 0;
	m_iTotalProgress = 15;
	m_iProgressCount = 0;
	m_bLoadStopFlag = FALSE;
	m_iDxfVersion = 0;
	m_lFileLength = 0;
	m_iMaxLayerNumber = 0;
	m_iColorNumber = 255;
	if (m_pLayerName != NULL) 
	{ 
		delete[] m_pLayerName; 
		m_pLayerName = NULL; 
	}
	for(int i=0;i<300;i++)
	{
		m_iLayerColor[i]=0;
	}
}


int CFileDxf::OpenEntityFileSave(void)
{
	char path[1000];		/** path local variable 01*/
	CString str;			/** str local variable 02*/

	wsprintfA(path, "%S\\entity.dat", m_strEntityFolder); 
	if((m_pFileCoord =_fsopen(path, "w",_SH_DENYNO))==NULL)
		return 0;

	return 1;
}


bool CFileDxf::FindDxfHeader(void)
{
	int		g_code;				/** g_code local variable 01*/
	int		response;			/** response local variable 02*/
	bool	continue_flag;		/** continue_flag local variable 03*/

	// HEADER SECTION
	if (FindDxfEntry(0, _T("SECTION")) == false) 
		return false;
	if (FindDxfEntry(2, _T("HEADER")) == false) 
		return false;

	continue_flag = true;
	while (continue_flag)
	{
		g_code = GetLine(MemoryBuf);
		switch (g_code)
		{
		case DXF_ERR : 
			return false;
		case 0 :	// file section mark
			if (MemoryBuf == "EOF") 	/* End of File. Exit loop */
			{
				return false;
			}
			else if (MemoryBuf == "ENDSEC") /* Section End. Exit loop */
			{ 
				continue_flag = false; 
			}
			break;
		case 9 :
			if (MemoryBuf == "$ACADVER")	/* Found Version String */
			{
				GetLine(MemoryBuf);
				response = CheckDxfVersion(MemoryBuf);
				if(response == false) 
				{
					return false;
				}
			}
			break;
		}
	}

	return true;
}


void CFileDxf::AllocDxfBuffer(void)
{
	m_pLineType = new tagLineTYPE[50];
	m_pLayer = new tagLAYER[100];
	memset(LPVOID(m_pLineType), 0, sizeof(tagLineTYPE)*50);
	memset(LPVOID(m_pLayer), 0, sizeof(tagLAYER)*50);
	m_iLineTYPEConut=0;
	m_iLayerCount=0;
}


bool CFileDxf::FindDxfTable(void)
{
	bool continue_flag;	/** continue_flag local variable 01*/
	bool t_flag;		/** t_flag local variable 02*/
	int	g_code;			/** g_code local variable 03*/
	int	t_code;			/** t_code local variable 04*/
	int	table;			/** table local variable 05*/
	int LayerCnt = 0;
	CString strG1, strG2, strG3, strG4;

	strG1 = "G1"; 
	strG2 = "G2";
	strG3 = "G3";
	strG4 = "G4";

	// TABLES SECTION
	m_iLineTYPEConut=0;
	m_iLayerCount=0;

	table = 0;
	t_flag = false;
	continue_flag = true;

	if (FindDxfEntry(0, _T("SECTION")) == false) 
		return false;
	if (FindDxfEntry(2, _T("TABLES")) == false) 
		return false;

	while (continue_flag)
	{
		g_code = GetLine(MemoryBuf);
		switch (g_code)
		{
		case DXF_ERR : return false;
		case 0 :	// file section mark
			if (MemoryBuf == "EOF")		/* End of file. Exit loop */
			{
				return false;
			}
			else if (MemoryBuf == "ENDSEC") /* End Section. Exit loop */
			{ 
				continue_flag = false; 
				break; 
			}
			else if (MemoryBuf == "ENDTAB") /* End Table */
			{ 
				t_flag = false; 
				continue; 
			}
			else if (MemoryBuf == "TABLE") 	/* Start Table */
			{ 
				t_flag = true; 
			}
			else if (MemoryBuf == "LTYPE" && t_flag == true && table == 1)
			{
				m_pLineType[m_iLineTYPEConut].Kind = -1;
				do
				{
					t_code = GetLine(MemoryBuf);
					if (t_code == 2)
					{
						wsprintfA(m_pLineType[m_iLineTYPEConut].ID, "%S", MemoryBuf); 
						if (m_pLineType[m_iLineTYPEConut].Kind != -1) 
						{ 
							m_iLineTYPEConut++; 
							break; 
						}
					}
					else if (t_code == 73)
					{
						switch (_ttoi(MemoryBuf))
						{
						case 0 : m_pLineType[m_iLineTYPEConut].Kind = PS_SOLID;		break;
						case 1 : m_pLineType[m_iLineTYPEConut].Kind = PS_DASH;		break;
						case 2 : m_pLineType[m_iLineTYPEConut].Kind = PS_DASHDOT;	break;
						case 3 : m_pLineType[m_iLineTYPEConut].Kind = PS_DOT;		break;
						default: m_pLineType[m_iLineTYPEConut].Kind = PS_SOLID;		break;
						}
						if (m_pLineType[m_iLineTYPEConut].ID[0]) 
						{ 
							m_iLineTYPEConut++; 
							break; 
						}
					}
					else if (t_code == 0) break;
				} while (t_code != DXF_ERR);
			}
			else if (MemoryBuf == "LAYER" && t_flag == true && table == 2)
			{
				m_pLayer[LayerCnt].ID[0] = NULL;
				m_pLayer[LayerCnt].color = 0;
				m_pLayer[LayerCnt].line_type = -1;
				m_pLayer[LayerCnt].onoff = -1;
				do
				{
					t_code = GetLine(MemoryBuf);
					m_iLayerPointCount[LayerCnt] = 0;
					if (t_code == 2)
					{
						wsprintfA(m_pLayer[LayerCnt].ID, "%S", MemoryBuf); 
						if ( m_pLayer[LayerCnt].color && m_pLayer[LayerCnt].line_type != -1 && m_pLayer[LayerCnt].onoff != -1)
						{
							LayerCnt++;
							break;
						}
					}
					else if (t_code == 6)
					{
						for (int i=0; i<m_iLineTYPEConut; i++) 
						{
							if (m_pLineType[i].ID == MemoryBuf) 
							{ 
								m_pLayer[LayerCnt].line_type = m_pLineType[i].Kind; 
								break; 
							}
						}
						if (m_pLayer[LayerCnt].line_type == -1) 
							break;

						if(m_pLayer[LayerCnt].ID == strG1)m_nInspectionCnt++;
						if(m_pLayer[LayerCnt].ID == strG2)m_nInspectionCnt++;
						if(m_pLayer[LayerCnt].ID == strG3)m_nInspectionCnt++;
						if(m_pLayer[LayerCnt].ID == strG4)m_nInspectionCnt++;

						if (m_pLayer[LayerCnt].ID[0] && m_pLayer[LayerCnt].color && m_pLayer[LayerCnt].onoff != -1)
						{
							LayerCnt++;
							break;
						}
					}
					else if (t_code == 62)
					{
						m_pLayer[LayerCnt].color = _ttoi(MemoryBuf);
						m_iLayerColor[LayerCnt]=_ttoi(MemoryBuf);
						if (m_pLayer[LayerCnt].color < 0)
						{
							m_pLayer[LayerCnt].onoff = 1;	// OFF
							m_pLayer[LayerCnt].color *= -1;	// 양수로
						}
						else 
						{
							m_pLayer[LayerCnt].onoff = 0;	// ON
						}

						if (m_pLayer[LayerCnt].ID[0] && m_pLayer[LayerCnt].line_type != -1)
						{
							LayerCnt++;
							break;
						}

						m_iColorNumber = m_pLayer[LayerCnt].color;
					}
					else if (t_code == 0) 
					{
						break;
					}
				} while (t_code != DXF_ERR);
			}
			break;
		case 2 :
			if (MemoryBuf == "LTYPE") 
				table = 1;
			else if (MemoryBuf == "LAYER") 
				table = 2;
			else if (MemoryBuf == "STYLE") 
				table = 3;

			break;
// 		case 70 :
// 			if (table == 2) 
// 			{
// 				m_iLayerCount = _ttoi(MemoryBuf);
// 			}
// 			break;
		}
	}

	m_iLayerCount = LayerCnt;
	return true;
}

bool CFileDxf::FindDxfEntry(int grp, LPCTSTR f_str)
{
	int		g_code;			/** g_code local variable 01*/

	do
	{
		g_code = GetLine(MemoryBuf);
		if (grp == g_code && MemoryBuf == f_str) 
		{
			return true;
		}
	} while(g_code != DXF_ERR);

	return false;
}


bool CFileDxf::FindDxfEntry(int grp, CString *str)
{
	int		g_code;		/** g_code local variable 01*/
	do
	{
		g_code = GetLine(*str);
		if (grp == g_code) 
		{
			return true;
		}
	} while(g_code != DXF_ERR);
	return false;
}


int CFileDxf::GetVersion(void)
{
	return m_iDxfVersion;
}


int CFileDxf::GetLine(CString& buf)
{
	int	group_code;			/** group_code local variable 01*/
	char line[1000];		/** line local variable 02*/

	/* Get group code line */
	if (fgets(line,200,m_pFileLoad) == false) 
		return DXF_ERR;

	buf.Format(_T("%S"),line);

	group_code = _ttoi(MemoryBuf);

	/* Get next line */
	if (fgets(line,200,m_pFileLoad) == false) 
		return DXF_ERR;

// 	if(line == AcDbEntity)
// 	{
// 		if (fgets(line,100,m_pFileLoad) == false) 
// 			return DXF_ERR;
// 		if (fgets(line,100,m_pFileLoad) == false) 
// 			return DXF_ERR;
// 		if (fgets(line,100,m_pFileLoad) == false) 
// 			return DXF_ERR;
// 	}

	buf = line;
	buf.TrimLeft();
	buf.TrimRight();

	return (group_code);
}

bool CFileDxf::DrawPoint(void)
{
	int	g_code;		/** g_code local variable 01*/
	int	sum = 3;	/** sum local variable 02*/
	double x=0;		/** x local variable 03*/
	double y=0;		/** y local variable 04*/
	CString layer;	/** layer local variable 05*/

	do
	{
		if(m_bLoadStopFlag == TRUE)
			return 0;

		g_code = GetLine(MemoryBuf);

		/* Classify group code */
		if (g_code == 8)  
		{ 
			layer = MemoryBuf; 
		}
		else if (g_code == 10) 		/* X Point */
		{ 
			x = _ttof(MemoryBuf);	
			sum--; 
		}
		else if (g_code == 20) 		/* Y Point */
		{ 
			y = _ttof(MemoryBuf);	
			sum--;
			break;
		}
		else if (g_code == 62) 		/* Color */
		{ 
			m_iColorNumber = _ttoi(MemoryBuf); 
			sum--; 
			bLoadColorFlag=true; 
		}
		else if (g_code == 0)  
		{ 
			break;
		}
	} while(sum);

	/* Classify Position type */
	if(layer == L"G1")		/* Inspection Position 1 */
	{
		dInspPosX[0] = x;
		dInspPosY[0] = y;
	}
	else if(layer == L"G2")		/* Inspection Position 2 */
	{
		dInspPosX[1] = x;
		dInspPosY[1] = y;
	}
	else if(layer == L"G3")		/* Inspection Position 3 */
	{
		dInspPosX[2] = x;
		dInspPosY[2] = y;
	}
	else if(layer == L"G4")		/* Inspection Position 4 */
	{
		dInspPosX[3] = x;
		dInspPosY[3] = y;
	}
	else
	{
		/* Normal Position */
		int num = FindLayerNumber(layer);
		m_iLayerPointCount[num]++;

		if(bLoadColorFlag==false)
		{
			m_iColorNumber=m_iLayerColor[num];
		}

		/* Save point data to entity file */
		SavePoint(layer, m_iColorNumber, x, y);
	}

	return true;
}


bool CFileDxf::DrawLine(void)
{

	int count=0; ///추가부분
	int	g_code;
	int sum = 5;
	double sx=0;
	double sy=0;
	double ex=0;
	double ey=0;
	CString layer;

	do
	{
		if(m_bLoadStopFlag == TRUE)
			return 0;

		g_code = GetLine(MemoryBuf);

		if (g_code == 8) 
		{
			layer = MemoryBuf;
		}
		else if (g_code == 10) 
		{
			sx = _ttof(MemoryBuf);	
			sum--; 
		}
		else if (g_code == 20) 
		{
			sy = _ttof(MemoryBuf);	
			sum--; 
		}
		else if (g_code == 11)
		{ 
			ex = _ttof(MemoryBuf);	
			sum--;
		}
		else if (g_code == 21) 
		{
			ey = _ttof(MemoryBuf);	
			sum--; 
			break;
		}
		else if (g_code == 62) 
		{
			m_iColorNumber = _ttoi(MemoryBuf);	
			sum--; 
			bLoadColorFlag=true;
		}
		else if (g_code == 0)  
		{ 
			break;
		}
	}while(sum);

	int num = FindLayerNumber(layer);
	m_iLayerPointCount[num]++;
	if(bLoadColorFlag==false)
	{
		m_iColorNumber=m_iLayerColor[num];
	}

	/* Save Line data to entity file */
	SaveLine(layer, m_iColorNumber, sx, sy, ex, ey);

	return true;
}


bool CFileDxf::DrawCircle(void)
{
	int	g_code;
	int sum = 4;
	double x,y,r;
	CString layer;

	do
	{
		if(m_bLoadStopFlag == TRUE)
			return 0;

		g_code = GetLine(MemoryBuf);

		if (g_code == 8) 
		{
			layer = MemoryBuf;
		}
		else if (g_code == 10) 
		{
			x = _ttof(MemoryBuf);	
			sum--; 
		}
		else if (g_code == 20) 
		{
			y = _ttof(MemoryBuf);
			sum--; 
		}
		else if (g_code == 40)
		{
			r = _ttof(MemoryBuf);
			sum--; 
			break;
		}
		else if (g_code == 62) 
		{
			m_iColorNumber = _ttoi(MemoryBuf);
			sum--; 
			bLoadColorFlag=true;
		}
		else if (g_code == 0)  
		{ 
			break;
		}
	} while(sum);

	int num = FindLayerNumber(layer);
	m_iLayerPointCount[num]++;

	if(bLoadColorFlag==false)
	{
		m_iColorNumber=m_iLayerColor[num];
	}

	SaveCircle(layer,m_iColorNumber,x,y,r);

	return true;
}


bool CFileDxf::DrawEllipse(void)
{
	int	g_code;
	int sum = 7;
	double x,y,sx,sy,ratio,sa,ea;
	CString layer;

	do
	{
		if(m_bLoadStopFlag == TRUE)
			return 0;

		g_code = GetLine(MemoryBuf);

			if (g_code == 8) 
		{
			layer = MemoryBuf;
		}
		else if (g_code == 10)			//cX
		{
			x = _ttof(MemoryBuf);	
			sum--; 
		}
		else if (g_code == 20)			//cX
		{
			y = _ttof(MemoryBuf);	
			sum--; 
		}
		else if (g_code == 11)			//Start X
		{
			sx = _ttof(MemoryBuf);	
			sum--; 
		}
		else if (g_code == 21)			//Start Y
		{
			sy = _ttof(MemoryBuf);
			sum--; 
		}
		else if (g_code == 40)			//r ratio x
		{
			ratio = _ttof(MemoryBuf);
			sum--; 
		}
		else if (g_code == 41)			//startAngle Rad
		{
			sa = _ttof(MemoryBuf);
			sum--; 
		}
		else if (g_code == 42)			//EndAngle Rad
		{
			ea = _ttof(MemoryBuf);
			sum--; 
			break;
		}
		else if (g_code == 0)  
		{ 
			break;
		}
	} while(sum);

	int num = FindLayerNumber(layer);
	m_iLayerPointCount[num]++;

	if(bLoadColorFlag==false)
	{
		m_iColorNumber=m_iLayerColor[num];
	}

	SaveEllipse(layer,m_iColorNumber,x,y,sx,sy,ratio,sa,ea);

	return true;
}


bool CFileDxf::DrawArc(void)
{
	int	g_code, sum = 6;
	double x,y,r,sa,ea;
	CString layer;

	do
	{
		if(m_bLoadStopFlag == TRUE)
			return 0;

		g_code = GetLine(MemoryBuf);

		if (g_code == 8)
		{
			layer = MemoryBuf;
		}
		else if (g_code == 10) 
		{ 
			x = _ttof(MemoryBuf);
			sum--; 
		}
		else if (g_code == 20)
		{ 
			y = _ttof(MemoryBuf);	
			sum--;
		}
		else if (g_code == 40) 
		{ 
			r = _ttof(MemoryBuf);	
			sum--;
		}
		else if (g_code == 50) 
		{
			sa = _ttof(MemoryBuf);
			sum--; 
		}
		else if (g_code == 51) 
		{
			ea = _ttof(MemoryBuf);
			sum--; 
			break;
		}
		else if (g_code == 62)
		{ 
			m_iColorNumber = _ttoi(MemoryBuf);
			sum--;
			bLoadColorFlag=true;
		}
		else if (g_code == 0)  
		{ 
			break;
		}
	} while(sum);

	int num = FindLayerNumber(layer);
	m_iLayerPointCount[num]++;

	if(bLoadColorFlag==false)
	{
		m_iColorNumber=m_iLayerColor[num];
	}

	/* Save Line data to entity file */
	SaveArc(layer, m_iColorNumber, x, y, r, sa, ea);

	return true;
}

bool CFileDxf::DrawPolyLine(void)
{
	CString	str,layer;
	int	g_code;
	int t_code;
	int sum=1;
	BOOL close_flag=FALSE,s_flag=TRUE,arc_flag=FALSE,get_flag=FALSE;
	double bulge=0.0000;
	double start_x,start_y,end_x,end_y,angle,real_x,real_y,ox,oy,sx,sy,ex,ey;

	do
	{
		if(get_flag==FALSE) 
			g_code = GetLine(MemoryBuf);
			
		if (g_code == 66) 
		{
			if (_ttoi(MemoryBuf) != 1) 
				return FALSE; 
		}
		else if (g_code == 70) 
		{
			if (_ttoi(MemoryBuf) == 1) 
				close_flag=TRUE; 
		}
		else if (g_code == 0 && MemoryBuf=="VERTEX")
		{
			sum = 1;
			do
			{	
				t_code = GetLine(MemoryBuf);

				if (t_code == 0 && MemoryBuf=="SEQEND") 
					break;

				if (t_code == 0 && MemoryBuf=="VERTEX") 
				{
					get_flag = TRUE;
					arc_flag = FALSE;
					g_code = t_code;
					break; //escape
				}

				if(t_code == 8)
				{
					layer = MemoryBuf;
				}
				else if (t_code == 62) // x-coord 
				{ 
					m_iColorNumber = _ttoi(MemoryBuf);
					bLoadColorFlag=TRUE;
				}
				else if (t_code == 10) // x-coord 
				{ 
					real_x = _ttof(MemoryBuf);
				}
				else if (t_code == 20) // y-coord
				{ 
					real_y = _ttof(MemoryBuf);
				}
				else if (t_code == 30) // bulge-angle
				{ 
					if(arc_flag == TRUE)
					{
						end_x = real_x;
						end_y = real_y;
						SavePolyArc(layer,m_iColorNumber,start_x,start_y,end_x,end_y,angle);
						arc_flag = FALSE;
						sx = end_x;
						sy = end_y;
					}
					else
					{
						if(s_flag)
						{
							ox = real_x; 
							oy = real_y;
							sx = real_x;
							sy = real_y;
							s_flag = FALSE;
						}
						else
						{
							ex = real_x;
							ey = real_y;

							int num = FindLayerNumber(layer);
							m_iLayerPointCount[num]++;	

							if(bLoadColorFlag==false)
							{
								m_iColorNumber=m_iLayerColor[num];
							}
											
							SaveLine(layer,m_iColorNumber,sx,sy,ex,ey);

							sx = ex;
							sy = ey;

							sum = 1;
						}
					}
				}
				else if (t_code == 42) // bulge-angle
				{ 
					bulge = _ttof(MemoryBuf);
					angle = (atan(bulge)*180/PHI)*4;
					
					start_x = real_x;
					start_y = real_y;
					arc_flag = TRUE;
					get_flag = FALSE;
					sum=0;
				}
			} while(sum);
		}
	} while(MemoryBuf != "SEQEND");

	if (close_flag)
	{
		ex = ox;
		ey = oy;

		if(arc_flag)
		{
			SavePolyArc(layer,m_iColorNumber,sx,sy,ex,ey,angle);
		}
		else
		{
			int num = FindLayerNumber(layer);
			m_iLayerPointCount[num]++;	

			if(bLoadColorFlag==false)
			{
				m_iColorNumber=m_iLayerColor[num];
			}

			SaveLine(layer,m_iColorNumber,sx,sy,ex,ey);
		}
	}

	return TRUE;
}

bool CFileDxf::DrawPolyLine2000(void)
{
	CString	str,layer;
	int	g_code, t_code, sum=1;
	BOOL close_flag=FALSE,s_flag=TRUE,arc_flag=FALSE,get_flag=FALSE,last_flag=FALSE,poly_flag=FALSE;
	double bulge=0.0000;
	double start_x,start_y,end_x,end_y,angle,real_x,real_y,ox,oy,sx,sy,ex,ey;

	do
	{
		poly_flag=TRUE;

		if( get_flag == FALSE )
			g_code = g_code = GetLine(MemoryBuf);

		if (g_code == 100 && MemoryBuf=="AcDbEntity")
		{
			sum = 1;
			close_flag=FALSE;s_flag=TRUE;arc_flag=FALSE;get_flag=FALSE;last_flag=FALSE;
			do
			{

				t_code = GetLine(MemoryBuf);
				if (t_code == 0)
				{
					if(last_flag)
					{
						int num = FindLayerNumber(layer);
						m_iLayerPointCount[num]++;	

						if(bLoadColorFlag==false)
						{
							m_iColorNumber=m_iLayerColor[num];
						}

						SaveLine(layer,m_iColorNumber,sx,sy,ox,oy);
						last_flag=FALSE;
					}
					ex = ox;
					ey = oy;

					sum = 0;
					s_flag = TRUE;
					arc_flag = FALSE;
					poly_flag=FALSE;
					m_bIsPolyLoaded = TRUE;
					break;
				}


				if (t_code==70 && MemoryBuf =="1")
					last_flag=TRUE;

				if(t_code == 8)
				{
					layer = MemoryBuf;
				}
				else if (t_code == 62) // x-coord 
				{
					bLoadColorFlag=TRUE;
					m_iColorNumber = _ttoi(MemoryBuf);
				}
				else if (t_code == 10) // x-coord 
				{
					real_x = _ttof(MemoryBuf);
				}
				else if (t_code == 20) // y-coord
				{ 
					real_y = _ttof(MemoryBuf);

					if(s_flag)
					{
						ox = real_x; 
						oy = real_y;
						sx = real_x;
						sy = real_y;
						s_flag = FALSE;
					}
					else if(arc_flag==FALSE && s_flag==FALSE)
					{
						ex = real_x;
						ey = real_y;

						int num = FindLayerNumber(layer);
						m_iLayerPointCount[num]++;	

						if(bLoadColorFlag==false)
						{
							m_iColorNumber=m_iLayerColor[num];
						}

						SaveLine(layer,m_iColorNumber,sx,sy,ex,ey);

						sx = ex;
						sy = ey;

					}
					else if(arc_flag == TRUE && s_flag==FALSE)
					{
						end_x = real_x;
						end_y = real_y;

						SavePolyArc(layer,m_iColorNumber,start_x,start_y,end_x,end_y,angle);

						arc_flag = FALSE;
						sx = end_x;
						sy = end_y;
					}
				}
				else if (t_code == 42) // 
				{ 
					bulge = _ttof(MemoryBuf);
					angle = (atan(bulge)*180/PHI)*4;
					
					if(angle<0) 
					{
						//circle_direction = CW;
						start_x = real_x;
						start_y = real_y;
						arc_flag = TRUE;
						get_flag = FALSE;
					}
					else
					{
						//circle_direction = CCW;
						start_x = real_x;
						start_y = real_y;
						arc_flag = TRUE;
						get_flag = FALSE;
					}
					sum=1;
				}
			} while(sum);
		}
	} while(poly_flag);

	//MemoryBuf = poly_flag;

	return true;
}

bool CFileDxf::DrawText(void)
{
	return true;
}

bool CFileDxf::DrawSolid(void)
{
	return true;
}

bool CFileDxf::DrawInsert(void)
{
	return true;
}


int CFileDxf::SortLayerNumber(void)
{
// 	CString* pStr;	/** pStr local variable 01*/
// 	int* Cnt;		/** Cnt local variable 02*/
// 	tagLAYER t;		/** t local variable 03*/
// 	CString str;	/** str local variable 04*/

	m_iMaxLayerNumber = m_iLayerCount;

// 	pStr = new CString[m_iMaxLayerNumber];
// 	Cnt = new int[m_iMaxLayerNumber];
// 
// 	/* Save layer data to temporary buffer */
// 	for(int k=0;k<m_iMaxLayerNumber;k++)	/** k local variable 05*/
// 	{
// 		if(m_bLoadStopFlag == TRUE)
// 		{
// 			delete [] pStr;
// 			delete [] Cnt;
// 			return 0;
// 		}
// 
// 		pStr[k] = m_pLayer[k].ID;
// 
// 		pStr[k] = pStr[k].Left(1);
// 
// 		Cnt[k] = _ttoi(pStr[k]);
// 	}
// 
// 	/* Sort */
// 	for(int a=0;a<m_iMaxLayerNumber;a++)		/** a local variable 06*/
// 	{
// 		for(int b=a+1;b<m_iMaxLayerNumber;b++)	/** b local variable 07*/
// 		{
// 			if(m_bLoadStopFlag == TRUE)
// 			{
// 				delete [] pStr;
// 				delete [] Cnt;
// 				return 0;
// 			}
// 
// 			if(Cnt[a] > Cnt[b])
// 			{
// 				t = m_pLayer[a];
// 				m_pLayer[a] = m_pLayer[b];
// 				m_pLayer[b] = t;
// 
// 				str = m_pLayer[a].ID;
// 				Cnt[a] = _ttoi(str.Left(1));
// 
// 				str = m_pLayer[b].ID;
// 				Cnt[b] = _ttoi(str.Left(1));
// 			}
// 		}
// 	}
// 	delete [] pStr;
// 	delete [] Cnt;

	if (m_pLayerName != NULL) 
	{ 
		delete[] m_pLayerName; 
		m_pLayerName = NULL; 
	}

	m_pLayerName = new CString[m_iMaxLayerNumber];

	for(int k=0;k<m_iMaxLayerNumber;k++)	/** k local variable 08*/
	{
		if(m_bLoadStopFlag == TRUE)
			return 0;

		m_pLayerName[k] = m_pLayer[k].ID;
	}

	int temp = 0;
	for(int i=0;i<m_iLayerCount;i++)
	{
		if(m_iLayerPointCount[i]==0)
		{
			temp++;
		}
	}
	m_iLayerCount -= temp;

	return 1;
}

void CFileDxf::CloseEntityFileSave(void)
{
	fclose(m_pFileCoord);
}


int CFileDxf::MakeCommonEntityFile(void)
{
	int nFileLength;
	int nCnt;
	char read_path[1000];		/** read_path local variable 01*/
	char write_path[1000];		/** write_path local variable 02*/
	CString path;				/** path local variable 03*/
	CString str;				/** str local variable 04*/

	wsprintfA(write_path, "%S\\entity.dat", m_strCommonFolder); 
	wsprintfA(read_path, "%S\\entity.dat", m_strEntityFolder); 
	path = read_path;

	CStdioFile read;			/** read local variable 05*/

	if(read.Open(path,CFile::modeRead|CFile::shareDenyNone|CFile::typeText)== false)
		return 0;

	if((m_pFileWrite =_fsopen(write_path, "w",_SH_DENYNO))==NULL)
	{
		read.Close();
		return 0;
	}

	nCnt= 0;
	nFileLength = (int)read.GetLength();

	fprintf(m_pFileWrite, "[EntityInformation]\n");

	fprintf(m_pFileWrite, "InspectionCount %d\n", m_nInspectionCnt);
	for(int i = 0; i < m_nInspectionCnt; i++)
	{
		fprintf(m_pFileWrite, "InspectionPoint%d X: %lf Y: %lf\n", i+1, dInspPosX[i], dInspPosY[i]);
	}
	
 	fprintf(m_pFileWrite, "EntityLength %d\n",m_lFileLength);
	fprintf(m_pFileWrite, "ToolCount %d\n",m_iLayerCount);
	for(int i=0; i<m_iMaxLayerNumber; i++)
	{
		CString strTemp;
		CString strMakeNoBlank = L"";

		strTemp.Format(L"%S", m_pLayer[i].ID);

		for(int strNum = 0; strNum < strTemp.GetLength(); strNum++)
		{
			TCHAR ch = strTemp.GetAt(strNum);

			if( ch == ' ')
				strMakeNoBlank += '_';
			else
				strMakeNoBlank += ch;
		}

		wsprintfA(m_pLayer[i].ID, "%S", strMakeNoBlank); 

		if(m_iLayerPointCount[i]!=0)
 			fprintf(m_pFileWrite, "Tool %d %s %d %.4f\n",i, m_pLayer[i].ID, m_iLayerPointCount[i], 0.10);
	}

	fprintf(m_pFileWrite, "%%\n");

	while(1)
	{
		if(m_bLoadStopFlag == TRUE)
		{
			read.Close();
			fclose(m_pFileWrite);
			return 0;
		}

		if(nCnt >= nFileLength) 
			break;

		read.ReadString(str);
		nCnt++;

		if(str == ""|| str == L"") 
		{
			break;
		}
		else 
		{
			fprintf(m_pFileWrite, "%S\n",str);
		}
	}
	read.Close();

	fclose(m_pFileWrite);

	return 1;
}

void CFileDxf::InitializeFileFolder(void)
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

	m_strEntityFolder = TempStr+_T("\\Coord");
	CreateDirectory(m_strEntityFolder, NULL);

	m_strEntityFolder = m_strEntityFolder+_T("\\Dxf");
	CreateDirectory(m_strEntityFolder, NULL);

	m_strEntityFolder = m_strEntityFolder+_T("\\Entity");
	CreateDirectory(m_strEntityFolder, NULL);
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

int CFileDxf::CheckDxfVersion(LPCTSTR strCode)
{
	CString Comp;		/** Comp local variable 01*/
	Comp = strCode;		

	if( Comp == "AC1006") m_iDxfVersion = 2; //R11 & R12
	else if( Comp == "AC1009") m_iDxfVersion = 3; //R13
	else if( Comp == "AC1012") m_iDxfVersion = 4; //R13
	else if( Comp == "AC1014") m_iDxfVersion = 5; //R14
	else if( Comp == "AC1015") m_iDxfVersion = 6; //ACAD2000
	else if( Comp == "AC1018") m_iDxfVersion = 7; //ACAD2004

	if(m_iDxfVersion == 7) 
		return 0;
	else 
		return 1;

	return 0;
}

void CFileDxf::SavePoint(CString i_strLayerName, int i_iColorNumber, double i_dPosX, double i_dPosY)
{
	CString strMakeNoBlank = L"";

	for(int strNum = 0; strNum < i_strLayerName.GetLength(); strNum++)
	{
		TCHAR ch = i_strLayerName.GetAt(strNum);

		if( ch == ' ')
			strMakeNoBlank += '_';
		else
			strMakeNoBlank += ch;
	}

	i_strLayerName = strMakeNoBlank;

	fwprintf(m_pFileCoord, _T("POINT %s %d %lf %lf\n"),i_strLayerName,i_iColorNumber,(i_dPosX),(i_dPosY));

	m_lFileLength++;
}

void CFileDxf::SaveLine(CString i_strLayerName, int i_iColorNumber, double i_dStartPosX, double i_dStartPosY, double i_dEndPosX, double i_dEndPosY)
{
	CString strMakeNoBlank = L"";

	for(int strNum = 0; strNum < i_strLayerName.GetLength(); strNum++)
	{
		TCHAR ch = i_strLayerName.GetAt(strNum);

		if( ch == ' ')
			strMakeNoBlank += '_';
		else
			strMakeNoBlank += ch;
	}

	i_strLayerName = strMakeNoBlank;

	fwprintf(m_pFileCoord, _T("LINE %s %d %.10f %.10f %.10f %.10f\n"), i_strLayerName, i_iColorNumber, (i_dStartPosX),(i_dStartPosY),(i_dEndPosX),(i_dEndPosY));

	m_lFileLength++;
}

void CFileDxf::SaveCircle(CString i_strLayerName, int i_iColorNumber, double i_dCenterPosX, double i_dCenterPosY, double i_dRadius)
{
	CString strMakeNoBlank = L"";

	for(int strNum = 0; strNum < i_strLayerName.GetLength(); strNum++)
	{
		TCHAR ch = i_strLayerName.GetAt(strNum);

		if( ch == ' ')
			strMakeNoBlank += '_';
		else
			strMakeNoBlank += ch;
	}

	i_strLayerName = strMakeNoBlank;

	double temp_a,temp_b;

	temp_a = i_dCenterPosX + i_dRadius;
	temp_b = i_dCenterPosY;

	fwprintf(m_pFileCoord, _T("CIRCLE %s %d %lf %lf %lf %lf %lf %lf %lf\n"),
		i_strLayerName,i_iColorNumber,(i_dCenterPosX),(i_dCenterPosY),(i_dRadius),(temp_a),(temp_b),(temp_a),(temp_b));

	m_lFileLength++;
}

void CFileDxf::SaveArc(CString i_strLayerName, int i_iColorNumber, double i_dCenterPosX, double i_dCenterPosY, double i_dRadius, double i_dStartAngle, double i_dEndAngle)
{
	CString strMakeNoBlank = L"";

	for(int strNum = 0; strNum < i_strLayerName.GetLength(); strNum++)
	{
		TCHAR ch = i_strLayerName.GetAt(strNum);

		if( ch == ' ')
			strMakeNoBlank += '_';
		else
			strMakeNoBlank += ch;
	}

	i_strLayerName = strMakeNoBlank;

	double ang,temp_a,temp_b,temp_c,temp_d;

	ang = i_dStartAngle * PHI / 180.;
	temp_a = i_dCenterPosX + (i_dRadius * cos(ang));
	temp_b = i_dCenterPosY + (i_dRadius * sin(ang));

	if( i_dStartAngle > i_dEndAngle) i_dEndAngle += 360;

	ang = i_dEndAngle * PHI / 180.;
	temp_c = i_dCenterPosX + (i_dRadius * cos(ang));
	temp_d = i_dCenterPosY + (i_dRadius * sin(ang));

	fwprintf(m_pFileCoord, _T("ARC %s %d %f %f %.16f %.16f %.16f %.16f %.16f %.16f %.16f\n"),i_strLayerName,i_iColorNumber,(i_dCenterPosX),(i_dCenterPosY),(i_dRadius),(i_dStartAngle),(i_dEndAngle),(temp_a),(temp_b),(temp_c),(temp_d));

	m_lFileLength++;
}

void CFileDxf::SaveEllipse(CString i_strLayerName, int i_iColorNumber, double i_dCenterPosX, double i_dCenterPosY, double i_dLongX, double i_dLongY, double i_dRatio, double sA, double eA)
{
	CString strMakeNoBlank = L"";

	for(int strNum = 0; strNum < i_strLayerName.GetLength(); strNum++)
	{
		TCHAR ch = i_strLayerName.GetAt(strNum);

		if( ch == ' ')
			strMakeNoBlank += '_';
		else
			strMakeNoBlank += ch;
	}

	i_strLayerName = strMakeNoBlank;

	double sa, ea;
	sa = sA * 180. / PHI;
	ea = eA * 180. / PHI;

	//if( i_dStartAngle > i_dEndAngle) i_dEndAngle += 360;

	fwprintf(m_pFileCoord, _T("ELLIPSE %s %d %lf %lf %lf %lf %.14f %.14f %.14f\n"),i_strLayerName,i_iColorNumber,(i_dCenterPosX),(i_dCenterPosY),(i_dLongX),(i_dLongY),(i_dRatio),(sa),(ea));

	m_lFileLength++;
}

void CFileDxf::SavePolyArc(LPCTSTR ly,int cr,double sx, double sy, double ex, double ey, double angle)
{
	CString strMakeNoBlank = L"";
	CString strTemp;

	strTemp.Format(L"%s", ly);

	for(int strNum = 0; strNum < strTemp.GetLength(); strNum++)
	{
		TCHAR ch = strTemp.GetAt(strNum);

		if( ch == ' ')
			strMakeNoBlank += '_';
		else
			strMakeNoBlank += ch;
	}

	ly = strMakeNoBlank;

	double chord,diameter,temp;
	double alpha1,alpha2,radius,temp_angle;
	double center_x,center_y,line;
	double temp_center_x,temp_center_y;
	double sa,ea;

	chord = sqrt(((sx-ex)*(sx-ex))+((sy-ey)*(sy-ey)));
	chord = chord/2;

	temp_angle = angle;
	if(angle < 0) temp_angle = angle*-1;

	if(temp_angle>180) temp_angle = 360 - temp_angle;

	alpha1 = temp_angle/2;
	alpha2 = 180-(90+alpha1);

	temp = cos(alpha2*PHI/180);

	diameter = chord/temp;

	temp_center_x = (sx+ex)/2;
	temp_center_y = (sy+ey)/2;

	line = sqrt((diameter*diameter)-(chord*chord));

	radius = angle*PHI /180;
	double X; X = cos(radius);
	double Y; Y = -sin(radius);
	double Z; Z = sin(radius);
	double W; W = cos(radius);
	double Ans1,Ans2;
	CString str;

	Ans1 = ((X-1)*((Z*sx)+(W*sy)-ey))+(Z*ex)-(Z*X*sx)-(Z*Y*sy);
	Ans2 = (W*X)-W-(Z*Y)-X+1;
	center_y = Ans1/Ans2;

	Ans1 = (X*sx)+(Y*sy)-(Y*center_y)-ex;
	Ans2 = X-1;
	center_x = Ans1/Ans2;

	//if(circle_direction == CW) //TEST 필요
	if(angle<0)
	{
		sa = Cal_Angle(center_x,center_y,ex,ey);
		ea = Cal_Angle(center_x,center_y,sx,sy);
	}
	else
	{
		sa = Cal_Angle(center_x,center_y,sx,sy);
		ea = Cal_Angle(center_x,center_y,ex,ey);
	}

	/* Normal Position */
	int num = FindLayerNumber(ly);
	m_iLayerPointCount[num]++;

	if(bLoadColorFlag==false)
	{
		m_iColorNumber=m_iLayerColor[num];
	}

	/* Save point data to entity file */
	SaveArc(ly,cr,center_x,center_y,diameter,sa,ea);
}




double CFileDxf::DoRound(double i_dNumber)
{
	i_dNumber =  floor(1000000.*(i_dNumber + 0.0000005))/1000000. ;
	return i_dNumber;
}


void CFileDxf::SetLoadStop(BOOL bStopFlag)
{
	m_bLoadStopFlag = bStopFlag;
}


void CFileDxf::SetProgress(int iNum)
{
	m_iTotalProgress = iNum;
}


int CFileDxf::GetProgressTotal(void)
{
	return m_iTotalProgress;
}


int CFileDxf::GetProgress(void)
{
	return m_iProgressCount;
}


int CFileDxf::FindLayerNumber(CString strLayerName)
{
	for(int j=0;j<m_iLayerCount;j++)
	{
		if(strLayerName == m_pLayer[j].ID)
		{
			return j;
		}
	}
	return 0;
}


double CFileDxf::Cal_Angle(double cx, double cy, double x, double y)
{
	double temp_x;
	double temp_y;
	double angle;

	temp_x = x-cx;
	temp_y = y-cy;

	angle = atan2(temp_y,temp_x) * 180 / PHI;

	return angle;
}

void CFileDxf::SortByLayerName(void)
{
	int nTempCount;
	CString strTempName;
	CString strLayerName[10];

	for(int i=0; i<m_iMaxLayerNumber; i++)
	{
		CString strTemp;
		CString strMakeNoBlank = L"";

		strTemp.Format(L"%S", m_pLayer[i].ID);

		for(int strNum = 0; strNum < strTemp.GetLength(); strNum++)
		{
			TCHAR ch = strTemp.GetAt(strNum);

			if( ch == ' ')
				strMakeNoBlank += '_';
			else
				strMakeNoBlank += ch;
		}

		wsprintfA(m_pLayer[i].ID, "%S", strMakeNoBlank); 

		strLayerName[i].Format(L"%S", m_pLayer[i].ID);
	}

	for(int i=0; i<m_iMaxLayerNumber; i++)
	{
		if(m_iLayerPointCount[i] <=0) continue;

		for(int j=i+1; j<m_iMaxLayerNumber; j++)
		{
			if(strLayerName[i] > strLayerName[j])
			{
				strTempName.Format(L"%S", m_pLayer[i].ID);
				nTempCount = m_iLayerPointCount[i];

				wsprintfA(m_pLayer[i].ID, "%s", m_pLayer[j].ID); 
				m_iLayerPointCount[i] = m_iLayerPointCount[j];

				wsprintfA(m_pLayer[j].ID, "%S", strTempName); 
				m_iLayerPointCount[j] = nTempCount;
			}
		}
	}
}
