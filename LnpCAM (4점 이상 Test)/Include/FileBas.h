/*********************************************************************************
 * Copyright(c) 2012 by Laser&Physics.
 * 
 * This software is copyrighted by, and is the sole property of Laser&Physics.
 * All rigths, title, ownership, or other interests in the software remain the
 * property of Laser&Physics. This software may only be used in accordance with
 * the corresponding license agreement. Any unauthorized use, duplication, 
 * transmission, distribution, or disclosure of this software is expressly 
 * forbidden.
 *
 * This Copyright notice may not be removed or modified without prior written
 * consent of Laser&Physics. Laser&Physics reserves the right to modify this 
 * software without notice.
 *
 * Laser&Physics.
 * KOREA 
 * http://www.Laser-Physics.co.kr
 *********************************************************************************/

/**
	*@class CFileBas
	*@brief This class tranlate to the bas file.
*/

/* made by kim seung hyuck */
/* made DLL by kim Junil */

#pragma once

class AFX_EXT_CLASS CFileBas :public CObject
{
public:
	CFileBas(void);
	~CFileBas(void);

	typedef struct _TOOLINFO
	{
		double	diameter;
		int		toolNum;
		int		countPoint;
		CString toolName;
	} TOOLINFO; 

	TOOLINFO*		toolInfo;

	FILE *m_fCoord;				// 최종적으로 해석된 Entity File을 저장할 File Pointer 변수
	
	int startEnd;
	int	countPoint;
	int countTool;

	/**
	* @var m_iCurrentDMode
	* @brief Save Current D Code
*/
	int m_iCurrentDMode;

/**
	* @var m_iCurrentGMode
	* @brief Save Current G Code
*/
	int m_iCurrentGMode;

/**
	* @var m_dCurrentPosX
	* @brief Save Current Pos X
*/
	double m_dCurrentPosX; 

/**
	* @var m_dCurrentPosY
	* @brief Save Current Pos Y
*/
	double m_dCurrentPosY; 

/**
	* @var m_lTrueLength
	* @brief Save True Length
*/
	long m_lTrueLength;

/**
	* @var m_strLayerName
	* @brief Save current layer name
*/
	CString	m_strLayerName;

/**
	* @var m_iLayerCount
	* @brief Count Layer
*/
	int	m_iLayerCount;

/**
	* @var m_iLayerNumber
	* @brief Total Layer Number
*/
	int	m_iLayerNumber;

/**
	* @var m_iDCode
	* @brief Save D code
*/
	int m_iDCode;

/**
	* @var m_iGCode
	* @brief Save G code
*/
	int m_iGCode; 

/**
	* @var m_iPCode
	* @brief Save P code
*/
	int m_iPCode; 

	DWORD basLength;			// 해석된 Entity의 갯수 저장 변수 

	double tempX;
	double tempY;			// 이전 좌표 저장

	CString modeProcess;
	CString modelName;
	CString m_strBasFolder;
	CString m_strDxfFolder;
	CString m_strCommonFolder;
	CString m_strEntityFolder;
	CString m_strCoordFolder;
	CString	buf;

	/**
	*@ fn Load(LPCTSTR FileName)
	*@ brief control the translate
	*@ param continue_flag the to retain translate
	*@ param startEnd the choice start or end
	*/
	bool Load(LPCTSTR FileName);

	/**
	*@ fn Find_Header(CStdioFile &OpenFile)
	*@ brief save the bas model
	*/
	bool Find_Header(CStdioFile &OpenFile);				// 미사용

	/**
	*@ fn Find_Point(CString coord)
	*@ brief extract x,y coord and save coord entity
	*@ param x,y currently x,y coord
	*@ param paceY is find the 'Y' place
	*/
	bool Find_Point(CString coord);

	/**
	*@ fn Init_Bas(void)
	*@ brief initialization
	*@ param countTool the to count the Tool number
	*@ param tempX,tempY the to uses the before coord
	*@ param basLength the to total coord number
	*@ param countPoint the to count coord number of tool
	*/
	void Init_Bas(void);

	/**
	*@ fn WRITE_POINT(CString tName, double cx, double cy)
	*@ biref write coordinate
	*@ param tempStr is receive the process mode
	*@ param [in,out]tName is tool name
	*@ param [in,out]cx,cy is X coordinates
	*@ param [out]countPoint is count of tool line length
	*@ param [out]basLength is count of total coordinate length
	*/
	void WRITE_POINT(CString tName, double cx, double cy);

	void WRITE_LINE(double dSX, double dSY, double dEX, double dEY);
	void WRITE_CIRCLE(double dX, double dY, double dR);
	void WRITE_ARC(double dX, double dY, double dR, double dSA, double dEA);
	/**
	*@ fn InitializeFileFolder(void)
	*@ brief create folder
	*/
	void InitializeFileFolder(void);

	int COORD_OPEN(void);
	int COORD_CLOSE(void);
	
	/**
	*@ fn Round(double initNum)
	*@ brief round off coordinate
	*@ param [in,out]initNum coordinate to the round off
	*@ return initNum
	*/
	double Round(double initNum);

	/**
	*@ fn Change_Coord(double originNum)
	*@ brief change the chipher of coordinate
	*@ return originNum
	*/
	double Change_Coord(double originNum);

	double m_dSX;
	double m_dSY;
	double m_dEX;
	double m_dEY;
	double m_dX;
	double m_dY;
	double m_dI;
	double m_dJ;

	void SaveTool(void);
	double Return_Diameter(int number);
	int ConvertDxf(void);

	void DeleteDrawBufferMem();
	void DeleteToolInfoMem();

	/**
	* @fn ApplySettingX(double iNum)
	* @brief Calibration X value
	* @return result of Calibration
	* @param [in] iNum before calibration value
	* @see None
*/
	double ApplySettingX(double iNum);

/**
	* @fn ApplySettingY(double iNum)
	* @brief Calibration Y value
	* @return result of Calibration
	* @param [in] iNum before calibration value
	* @see None
*/
	double ApplySettingY(double iNum);

	double Cal_Angle(double cx, double cy, double x, double y);

	int Abstract_GCode(CString strCmd, int G_pos,int D_pos,int X_pos,int Y_pos,int I_pos,int J_pos);
	double Abstract_ICode(CString strCmd,int I_pos,int J_pos,int D_pos);
	double Abstract_JCode(CString strCmd, int J_pos, int D_pos);
	double Abstract_YCode(CString strCmd, int Y_pos, int I_pos, int J_pos, int D_pos);
	double Abstract_XCode(CString strCmd, int X_pos,int Y_pos,int I_pos,int J_pos,int D_pos);

	bool Find_Line(CString strCoord);
	bool Find_Circcle(CString strCoord);
	bool Find_Arc(CString strCoord);
	bool Find_Entity(CString strEntity);

private:
		typedef struct  
		{
			CString entity;
			double sx, sy;
			double ex, ey;
			double px, py;
			double cx,cy;
			double r;
			double sa,ea;
		}GbrInfo;

		GbrInfo* DrawBuffer; // 버퍼 구조체 포인터 변수
		char drawing_name[1000];	// 해석후 Drawing name 저장 변수 

public:
	void InitEntity(void);
};

