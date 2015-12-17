/*********************************************************************************
 * Copyright(c) 2011,2012,2013 by CKM
 * 
 * This software is copyrighted by, and is the sole property of CKM.
 * All rigths, title, ownership, or other interests in the software remain the
 * property of CKM. This software may only be used in accordance with
 * the corresponding license agreement. Any unauthorized use, duplication, 
 * transmission, distribution, or disclosure of this software is expressly 
 * forbidden.
 *
 * This Copyright notice may not be removed or modified without prior written
 * consent of CKM.CKM reserves the right to modify this 
 * software without notice.
 *
 * CKM
 * KOREA 
 * 
 *********************************************************************************/
/**
 * @file  File_Drl.h
 * @brief . 
 *  This class is necessary to interpret the DRL file. 
 *  Analysis of the file and then produces results.   
 *  The file is 'Entity.dat'.
 * 
 * * 
 * @author : CKM
 * @date :2012.11.30
 * @version : 1.0
 * 
 * <b> Revision Histroy </b>
 * - 2012.11.30 First creation.
 * - 2013.03.11 Create DLL Module 
 */

#pragma once

class AFX_EXT_CLASS CFileDrl : public CObject
{
public:
	CFileDrl(void);
	~CFileDrl(void);

	public:
	/**
	* @struct STRUCT_POINT
	* @brief Define Point Information
*/
	typedef struct _STRUCT_POINT  
	{
		CString cstrX;
		CString cstrY;
		CString cstrR;
	}STRUCT_POINT;

	typedef struct _STRUCT_DXFPOINT  
	{
		CString cstrEntityName;
		float fX;
		float fY;
	}STRUCT_DXFPOINT;
	/**
	* @var m_Point
	* @brief Define variable of STRUCT_POINT
*/
	STRUCT_POINT m_Point;
	STRUCT_DXFPOINT* m_DxfPoint;
	/**
	* @struct STRUCT_TOOL
	* @brief Define TOOL Information
*/
	typedef struct _STRUCT_TOOL
	{
		int		nPointNumber;
		float	fDiameter;
		float	fZDepth;
		int		nRetractRate;
		int		nFeedRate;
		int		nHitMax;
		int		nSpindleRpm;
		int		nIndex;
		int		nToolName;
	}STRUCT_TOOL;
	/**
	* @var m_Tool
	* @brief Define variable of STRUCT_TOOL*
*/
	STRUCT_TOOL* m_Tool;
	/**
	* @struct STRUCT_HEAD
	* @brief Define HEAD Information
*/
	typedef struct _STRUCT_HEAD
	{ 
		BOOL	bInchMetric;
		BOOL	bLzTz;
		BOOL	bAbsoluteIncremental;
		BOOL	bDrillRoute;
		int		nVersion;
		int		nFormat;
		CString cstrPathName;
		int nPointNumber;
		int nToolNumber;
		int nUpper, nLower;
	}STRUCT_HEAD;
	/**
	* @var m_Head
	* @brief Define variable of STRUCT_HEAD
*/
	STRUCT_HEAD m_Head;
/**
	* @var m_nStartTime, m_nEndTime
	* @brief 로딩 시간 저장 변수
*/
	int m_nStartTime, m_nEndTime;
	int m_nLogCount;
/**
	* @var m_bRouteStartFlag
	* @brief 라우트모드시작플래그
*/
	BOOL m_bRouteLineStartFlag;
	BOOL m_bRouteCircleStartFlag;
	BOOL m_bRouteArcStartFlag;
/**
	* @var m_nToolIndex
	* @brief Tool Index 저장 변수
*/
	int m_nToolIndex;
/**
	* @var fResultX, fResultY, fIncResultX, fIncResultY
	* @brief Calc 완료 X,Y 결과값 저장 변수
*/
	float m_fResultX, m_fResultY;
/**
	* @var m_strEntityFolder[1000]
	* @brief Entity File을 저장할 File Path
*/
	CString m_strCoordFolder;
	CString m_strCommonFolder;
	CString m_strDrlFolder;
	CString m_strDxfFolder;
	CString m_strLogFolder;
	CString m_strEntityFolder;
/**
	* @var m_bIsLoaded
	* @brief 파일 로딩 성공 확인 플래그
*/
	BOOL m_bIsLoaded;
/**
	* @var m_fStartX, m_fStartY, m_fEndX, m_fEndY
	* @brief Line에 필요한 저장 변수
*/
	float m_fStartX, m_fStartY, m_fEndX, m_fEndY;
// Operations
public:
	/**
	* @fn Find_Header(CStdioFile &sf, CString &buf);
	* @brief Header를 검색 Tool number와 Point number 추출
	* @return BOOL
	* @see None
*/
	BOOL Find_Header(CStdioFile &sf, CString &buf);
/**
	* @fn Input_Information(CStdioFile &osf);
	* @brief entity.dat에 Information 입력
	* @return None
	* @see None
*/
	void Input_Information(CStdioFile &osf);
/**
	* @fn Input_Point(CStdioFile &sf, CStdioFile &osf, CString &buf);
	* @brief entity.dat에 Program 입력
	* @return None
	* @see None
*/
	BOOL Input_Program(CStdioFile &sf, CStdioFile &osf, CString &buf);
/**
	* @fn Input_Tool(CStdioFile &sf, CString &buf);
	* @brief TOOL 구조체에 Tool 정보 입력
	* @return None
	* @see None
*/
	void Input_Tool(CStdioFile &sf, CString &buf);
/**
	* @fn Is_Loaded();
	* @brief return m_bIsLoaded
	* @return BOOL
	* @see None
*/
	BOOL Is_Loaded();
/**
	* @fn Load_DrlFile(LPCTSTR fName);
	* @brief DRL 파일 로딩 함수
	* @return BOOL
	* @see None
*/
	BOOL Load_DrlFile(LPCTSTR fName);
/**
	* @fn Open_File();
	* @brief Dlg에서 파일 오픈
	* @return None
	* @see None
*/
	void Open_File();
/**
	* @fn Make_Tool();
	* @brief Tool 배열 동적 생성
	* @return BOOL
	* @see None
*/
	BOOL Make_Tool();
/**
	* @fn  Find_PointNumber(CStdioFile &sf, CString &buf);
	* @brief DRL 파일의 총 LINE Count
	* @return BOOL
	* @see None
*/
	BOOL Find_PointNumber(CStdioFile &sf, CString &buf, CStdioFile &osf);
/**
	* @fn Init_Tool();
	* @brief Init_Tool
	* @return None
	* @see None
*/
	void Init_Folder();
	void Init_Tool();
	void Init_Coord();
/**
	* @fn Init_Point();
	* @brief Init_Point
	* @return None
	* @see None
*/
	void Init_Point();
/**
	* @fn Init_Head();
	* @brief Init_Head
	* @return None
	* @see None
*/
	void Init_Head();
/**
	* @fn Calc_Coord(double nCoord);
	* @brief 좌표값 계산
	* @return double
	* @see None
*/
	float Calc_Coord(float duCoord);

	void Calc_AbsoluteIncremental(STRUCT_POINT &point);
	void Calc_DrillRouteMode(float fX, float fY);
	BOOL Calc_PositionXY(STRUCT_POINT &point, CString &buf);
	void Write_Program(CStdioFile &osf, CString &cstrOutString);
	/**
	* @fn Get_ToolNumber();
	* @brief return ToolNumber
	* @return int
	* @see None
*/
	int Get_ToolNumber();
	/**
	* @fn Get_PointNumber();
	* @brief return Get_PointNumber
	* @return int
	* @see None
*/
	int Get_PointNumber();
		/**
	* @fn Get_ToolName();
	* @brief return Get_ToolName
	* @return CString
	* @see None
*/
	CString Get_ToolName(int nIndexTool);
	/**
	* @fn Get_ToolDiameter();
	* @brief return Get_ToolDiameter
	* @return float
	* @see None
*/
	float Get_ToolDiameter(int nIndexTool);
/**
	* @fn Get_Format();
	* @brief return Get_Format
	* @return int
	* @see None
*/
	int Get_Format();
	/**
	* @fn Set_UpperLower();
	* @brief Set Upper, Lower
	* @return void
	* @see None
*/
	void Set_UpperLower(int nUpper, int nLower);

	double round(double value, int pos);
	void Delete_Tool();

	int Convert_Dxf();
	int MakeCommonEntityFile(void);
	void Init_DxfPoint();
	void Delete_DxfPoint();
	int m_nDxfPointIndex;
};

