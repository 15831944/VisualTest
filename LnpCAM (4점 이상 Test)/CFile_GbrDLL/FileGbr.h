#pragma once

#define LZO 0 // Gbr 헤더파일의 해석중 Code Length해석에서 Leading Zero를 정의하는 선언문
#define TZO 1 // Gbr 헤더파일의 해석중 Code Length해석에서 Tailing Zero를 정의하는 선언문
#define ABS 0 // Gbr 헤더파일의 해석중 Code Length해석에서 Absolute C.S.를 정의하는 선언문
#define INC 1 // Gbr 헤더파일의 해석중 Code Length해석에서 Increment C.S.를 정의하는 선언문
#define JUMP 0 // Gbr 좌표해석에서 Draw를 정의하는 선언문
#define MARK 1 // Gbr 좌표해석에서 Draw를 정의하는 선언문

class AFX_EXT_CLASS CFileGbr : public CObject
{
public:
	CFileGbr(void);
	~CFileGbr(void);

	/**
	* @fn LoadGbrFile(CString i_strFileName)
	* @brief Load file
	* @return result of file load
	* @param [in] i_strFileName Drl File Name.
	* @see InitializeGbrBuffer()
	* @see FindGbrHeader()
	* @see FindGbrCommandLine()
	* @see WriteEntityFile()
*/
	void SetLoadStop(BOOL bStopFlag);
	void SetProgress(int iNum);
	int GetProgressTotal(void);
	int GetProgress(void);
	bool LoadGbrFile(CString i_strFileName);
private:
/**
	* @fn InitializeFileFolder(void)
	* @brief Initialize File Folder
	* @return None
	* @see None
*/
	void InitializeFileFolder(void);

/**
	* @fn FindGbrCommandLine(void)
	* @brief Read command in Drl file
	* @return None
	* @see Abstract_GCode()
	* @see Abstract_XCode()
	* @see ApplySettingX()
	* @see Abstract_YCode()
	* @see ApplySettingY()
	* @see Abstract_ICode()
	* @see Abstract_JCode()
	* @see FindLayerNumber()
	* @see InsertBuffer()
*/
	void FindGbrCommandLine(void);

/**
	* @fn WriteEntityFile(void)
	* @brief Write Information to entity file
	* @return result of file load
	* @see None
*/
	bool WriteEntityFile(void);

/**
	* @fn Abstract_XCode(CString str, int X_pos,int Y_pos,int I_pos,int J_pos,int D_pos)
	* @brief Abstract X value in Command Line
	* @return X value
	* @param [in] strCmd Gbr Command Line
	* @param [in] X_pos X value Position
	* @param [in] Y_pos Y value Position
	* @param [in] I_pos I value Position
	* @param [in] J_pos J value Position
	* @param [in] D_pos D value Position
	* @see None
*/
	double Abstract_XCode(CString strCmd, int X_pos,int Y_pos,int I_pos,int J_pos,int D_pos);

	double Cal_Angle(double cx, double cy, double x, double y);

/**
	* @fn Abstract_YCode(CString strCmd, int Y_pos, int I_pos, int J_pos, int D_pos)
	* @brief Abstract Y value in Command Line
	* @return Y value
	* @param [in] strCmd Gbr Command Line
	* @param [in] Y_pos Y value Position
	* @param [in] I_pos I value Position
	* @param [in] J_pos J value Position
	* @param [in] D_pos D value Position
	* @see None
*/
	double Abstract_YCode(CString strCmd, int Y_pos, int I_pos, int J_pos, int D_pos);

/**
	* @fn Abstract_ICode(CString strCmd,int I_pos,int J_pos,int D_pos)
	* @brief Abstract I value in Command Line
	* @return I value
	* @param [in] strCmd Gbr Command Line
	* @param [in] I_pos I value Position
	* @param [in] J_pos J value Position
	* @param [in] D_pos D value Position
	* @see None
*/
	double Abstract_ICode(CString strCmd,int I_pos,int J_pos,int D_pos);

/**
	* @fn Abstract_JCode(CString strCmd,int J_pos,int D_pos)
	* @brief Abstract J value in Command Line
	* @return J value
	* @param [in] strCmd Gbr Command Line
	* @param [in] J_pos J value Position
	* @param [in] D_pos D value Position
	* @see None
*/
	double Abstract_JCode(CString strCmd,int J_pos,int D_pos);

/**
	* @fn Abstract_GCode(CString strCmd, int G_pos,int D_pos,int X_pos,int Y_pos,int I_pos,int J_pos)
	* @brief Abstract G value in Command Line
	* @return G value
	* @param [in] strCmd Gbr Command Line
	* @param [in] G_pos G value Position
	* @param [in] D_pos D value Position
	* @param [in] X_pos X value Position
	* @param [in] Y_pos Y value Position
	* @param [in] I_pos I value Position
	* @param [in] J_pos J value Position
	* @see None
*/
	int Abstract_GCode(CString strCmd, int G_pos,int D_pos,int X_pos,int Y_pos,int I_pos,int J_pos);

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

/**
	* @fn FindLayerNumber(CString i_strLayerName)
	* @brief Find Layer Number
	* @return Layer Number
	* @param [in] i_strLayerName Layer Name
	* @see None
*/
	int FindLayerNumber(CString i_strLayerName);

/**
	* @fn InsertBuffer(int iToolNum, double dX, double dY)
	* @brief Make drill data
	* @return None
	* @param [in] iToolNum Tool Number
	* @param [in] dX Pos X
	* @param [in] dY Pos Y
	* @param [in] fHoleSize Hole Size
	* @see None
*/
	void InsertBufferDrill(double dX, double dY, double dHoleSize);
	void InsertBufferCircle(double dX, double dY, double dR);
	void InsertBufferArc(double dX, double dY, double dR, double dSA, double dEA);
	void InsertBufferLine(double dSX, double dSY, double dEX, double dEY);
	void InsertBuffer(double dX, double dY);

/**
	* @fn DoRound(double i_dNumber)
	* @brief round the number
	* @return rounded number
	* @param [in] i_dNumber Original number
	* @see None
*/
	double DoRound(double i_dNumber);

/**
	* @fn InitializeGbrBuffer(void);
	* @brief Initialize all variable
	* @return None
	* @see None
*/
	void InitializeGbrBuffer(void);

/**
	* @fn FindDrlHeader(void)
	* @brief Find header in Gbr file
	* @return None
	* @see None
*/
	void FindGbrHeader(void);

/**
	* @fn GetLine(CString& buf)
	* @brief Loaded a word in file
	* @return result of file load
	* @param [Out] buf Loaded word
	* @see None
*/
	int GetLine(CString& buf);

/**
	* @struct GbrInfo
	* @brief Define Point Information
*/
	typedef struct 
	{
		CString strEntity;	/**< structure member 01 */
		CString strLayer;	/**< structure member 02 */
		double dCX;
		double dCY;
		double dR;
		double dSX;			/**< structure member 03 */
		double dSY;			/**< structure member 04 */
		double dEX;			/**< structure member 05 */
		double dEY;
		double dSA;
		double dEA;
		int		nColor;
	}GbrInfo;

/**
	* @var m_pDrawBuffer
	* @brief Define variable of GbrInfo
*/
	GbrInfo* pDrawBuffer; 

/**
	* @struct LAYER
	* @brief Define layer Information
*/
	typedef struct 
	{
		CString strLayerName;	/**< structure member 01 */
		float fHoleSize;		/**< structure member 02 */
		bool	bRectFlag;
		int		nEntityCnt;
	}LAYER;

/**
	* @var m_pLayer
	* @brief Define variable of LAYER
*/
	LAYER m_pLayer[100];


/**
	* @var GbrFileOut
	* @brief Gbr File Open
*/
	FILE* GbrFileOut;

/**
	* @var EntityFileOut
	* @brief Entity File Open
*/
	FILE* EntityFileOut;

/**
	* @var m_strCommandLine
	* @brief Define Command Line
*/
	CString m_strCommandLine;

/**
	* @var m_strEntityFolder
	* @brief Define Entity Folder
*/
	CString m_strEntityFolder; 

/**
	* @var m_strCommonFolder
	* @brief Define Common Folder
*/
	CString  m_strCommonFolder;

/**
	* @var MemoryBuf
	* @brief Define Command word
*/
	CString MemoryBuf;

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
	* @var m_lGbrLength
	* @brief Save Gbr File Length
*/
	long m_lGbrLength;

/**
	* @var m_lTrueLength
	* @brief Save True Job Length
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
	* @var m_iOmissionMode
	* @brief LZO or TZO
*/
	int m_iOmissionMode; 

/**
	* @var m_iCoordMode
	* @brief ABS or INC
*/
	int m_iCoordMode; 

/**
	* @var m_iXLower
	* @brief Lowest X Position
*/
	int m_iXLower;

/**
	* @var m_iXUpper
	* @brief Highest X Position
*/
	int m_iXUpper;

/**
	* @var m_iYLower
	* @brief Lowest Y Position
*/
	int m_iYLower; 

/**
	* @var m_iYUpper
	* @brief Highest Y Position
*/
	int m_iYUpper; 

/**
	* @var m_dX
	* @brief Current X value
*/
	double m_dX; 

/**
	* @var m_dY
	* @brief Current Y value
*/
	double m_dY; 

/**
	* @var m_dSX
	* @brief Current Start X value
*/
	double m_dSX; 

/**
	* @var m_dSY
	* @brief Current Start Y value
*/
	double m_dSY; 

/**
	* @var m_dEX
	* @brief Current End X value
*/
	double m_dEX; 

/**
	* @var m_dEY
	* @brief Current End Y value
*/
	double m_dEY; 

/**
	* @var m_dI
	* @brief Current I value
*/
	double m_dI; 

/**
	* @var m_dJ
	* @brief Current J value
*/
	double m_dJ; 

/**
	* @var m_fChangeUNIT
	* @brief INC = 25.4 ABS = 1
*/
	float m_fChangeUNIT; 

	BOOL m_bLoadStopFlag;
	int m_iTotalProgress;
	int m_iProgressCount;

/**
	* @var m_fChangeUNIT
	* @brief inpection Point count
*/
	int	m_nInspectionCnt;

	/**
	* @var m_fChangeUNIT
	* @brief inpection Point coord
*/
	double*	dInspPosX;
	double*	dInspPosY;

		/**
	* @var m_nLayerCnt
	* @brief Entity Cnt In Each Layer
*/
	int		m_nEntityCntInLayer[100]; 

};

