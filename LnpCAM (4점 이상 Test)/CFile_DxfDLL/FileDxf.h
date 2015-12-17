#pragma once
#define	DXF_ERR	-1
#include <share.h>

class AFX_EXT_CLASS CFileDxf :public CObject
{
public:
	CFileDxf(void);
	~CFileDxf(void);
/**
	* @fn LoadDxfFile(CString i_strFileName)
	* @brief Load file
	* @return result of file load
	* @param [in] i_strFileName Drl File Name.
	* @see DeleteDxfBuffer()
	* @see InitializeDxfVariable()
	* @see OpenEntityFileSave()
	* @see FindDxfHeader()
	* @see FindDxfTable()
	* @see FindDxfEntry()
	* @see GetVersion()
	* @see GetLine()
	* @see DrawPoint()
	* @see SortLayerNumber()
	* @see CloseEntityFileSave()
	* @see MakeCommonEntityFile()
*/
	bool LoadDxfFile(CString i_strFileName);

/**
	* @fn GetVersion(void)
	* @brief Get Dxf file Version
	* @return rDxf file Version
	* @see None
*/
	int GetVersion(void);
private:
/**
	* @fn DeleteDxfBuffer(void)
	* @brief Delete All variable
	* @return None
	* @see None
*/
	void DeleteDxfBuffer(void);

/**
	* @fn InitializeDxfVariable(void)
	* @brief Initialize all variable
	* @return None
	* @see None
*/
	void InitializeDxfVariable(void);

/**
	* @fn OpenEntityFileSave(void)
	* @brief Open EntityFile
	* @return result of file load
	* @see None
*/
	int OpenEntityFileSave(void);

/**
	* @fn CloseEntityFileSave(void)
	* @brief Close EntityFile
	* @return None
	* @see None
*/
	void CloseEntityFileSave(void);

/**
	* @fn MakeCommonEntityFile(void)
	* @brief Make common EntityFile
	* @return result of file load
	* @see None
*/
	int MakeCommonEntityFile(void);

/**
	* @fn FindDxfHeader(CStdioFile& sf)
	* @brief Find header in Dxf file
	* @return result of file load
	* @param [in] sf CStdioFile variable
	* @see FindDxfEntry()
	* @see GetLine()
	* @see CheckDxfVersion()
*/
	bool FindDxfHeader(void);

/**
	* @fn AllocDxfBuffer(void)
	* @brief Allocation memory
	* @return None
	* @see None
*/
	void AllocDxfBuffer(void);

/**
	* @fn FindDxfTable(CStdioFile& sf)
	* @brief Find table in dxf file
	* @return result of load
	* @param [in] sf CStdioFile variable
	* @see FindDxfEntry()
	* @see GetLine()
*/
	bool FindDxfTable(void);

/**
	* @fn FindDxfEntry(CStdioFile& sf, int grp, LPCTSTR f_str)
	* @brief Find DxfEntry
	* @return result of load
	* @param [in] sf CStdioFile variable
	* @param [in] grp Group code
	* @param [in] f_str Command word
	* @see GetLine()
*/
	bool FindDxfEntry(int grp, LPCTSTR f_str);

/**
	* @fn FindDxfEntry(CStdioFile& sf, int grp, CString *str)
	* @brief Find DxfEntry
	* @return result of load
	* @param [in] sf CStdioFile variable
	* @param [in] grp Group code
	* @param [in] str Command word
	* @see GetLine()
*/
	bool FindDxfEntry(int grp, CString *str);

/**
	* @fn GetLine(CString& buf)
	* @brief Loaded a word in file
	* @return result of file load
	* @param [Out] buf Loaded word
	* @see None
*/
	int GetLine(CString& buf);

/**
	* @fn InitializeFileFolder(void)
	* @brief Initialize File Folder
	* @return None
	* @see None
*/
	void InitializeFileFolder(void);

/**
	* @fn CheckDxfVersion(LPCTSTR strCode)
	* @brief Check use possibility 
	* @return Check use possibility 
	* @param [in] strCode Dxf file version command
	* @see None
*/
	int CheckDxfVersion(LPCTSTR strCode);

/**
	* @fn SavePoint(CString i_strLayerName, int i_iColoerNumber, double i_dPosX, double i_dPosY)
	* @brief Save Point in entity file
	* @return None
	* @param [in] i_strLayerName Layer Name
	* @param [in] i_iColoerNumber Coloer Number
	* @param [in] i_dPosX X Pos
	* @param [in] i_dPosY Y Pos
	* @see None
*/
	void SavePoint(CString i_strLayerName, int i_iColorNumber, double i_dPosX, double i_dPosY);

	void SaveLine(CString i_strLayerName, int i_iColorNumber, double i_dStartPosX, double i_dStartPosY, double i_dEndPosX, double i_dEndPosY);

	void SaveCircle(CString i_strLayerName, int i_iColorNumber, double i_dCenterPosX, double i_dCenterPosY, double i_dRadius);

	void SaveArc(CString i_strLayerName, int i_iColorNumber, double i_dCenterPosX, double i_dCenterPosY, double i_dRadius, double i_dStartAngle, double i_dEndAngle);

	void SavePolyArc(LPCTSTR ly,int cr,double sx, double sy, double ex, double ey, double angle);

	void SaveEllipse(CString i_strLayerName, int i_iColorNumber, double i_dCenterPosX, double i_dCenterPosY, double i_dLongX, double i_dLongY, double i_dRatio, double sA, double eA);


/**
	* @fn DoRound(double i_dNumber)
	* @brief round the number
	* @return rounded number
	* @param [in] i_dNumber Original number
	* @see None
*/
	double DoRound(double i_dNumber);

/**
	* @fn DrawPoint()
	* @brief Load group of point
	* @return result of file load
	* @see GetLine()
	* @see SavePoint()
*/

	bool DrawEllipse(void);

	bool DrawPoint(void);

	bool DrawLine(void);

	bool DrawCircle(void);

	bool DrawArc(void);

	bool DrawPolyLine(void);

	bool DrawPolyLine2000(void);

	bool DrawText(void);

	bool DrawSolid(void);

	bool DrawInsert(void);



/**
	* @fn SortLayerNumber(void)
	* @brief Save data a classified Layer number
	* @return None
	* @see None
*/
	int SortLayerNumber(void);

private:

/**
	* @struct tagLineTYPE
	* @brief Table Section Line Style
*/	
	typedef struct
	{
		char	ID[1000];	/**< structure member 01 */
		int		Kind;		/**< structure member 02 */
	} tagLineTYPE; 

/**
	* @var m_pLineType
	* @brief Define variable of tagLineTYPE
*/
	tagLineTYPE *		m_pLineType;
/**
	* @struct tagLAYER
	* @brief Table Section Layer Style
*/	
	typedef struct
	{
		char	ID[1000];	/**< structure member 01 */
		int		color;		/**< structure member 02 */
		int		line_type;	/**< structure member 03 */		
		int		onoff;		/**< structure member 04 */	
	} tagLAYER; 

/**
	* @var m_pLayer
	* @brief Define variable of tagLAYER
*/
	tagLAYER *		m_pLayer;

/**
	* @var m_iLayerColor
	* @brief Color number classified layer
*/
	int	m_iLayerColor[300];
	int	m_iLayerPointCount[300];

/**
	* @var m_iLineTYPEConut
	* @brief Count to LineTYPE
*/
	int	m_iLineTYPEConut;

/**
	* @var m_iLayerConut
	* @brief Count to Layer
*/
	int	m_iLayerCount;

/**
	* @var MemoryBuf
	* @brief Define Command word
*/
	CString MemoryBuf;

/**
	* @var bLoadColorFlag
	* @brief Line or Layer color
*/
	bool bLoadColorFlag;

/**
	* @var m_pLayerName
	* @brief Save Layer name
*/
	CString* m_pLayerName; 

/**
	* @var m_iMaxLayerNumber
	* @brief Total LayerNumber
*/
	int m_iMaxLayerNumber; 

/**
	* @var m_iDxfVersion
	* @brief Define Dxf file Version
*/
	int m_iDxfVersion; 

/**
	* @var m_lFileLength
	* @brief Length of point
*/
	long m_lFileLength;

/**
	* @var m_iColorNumber
	* @brief Current Color Number
*/
	int m_iColorNumber;

/**
	* @var m_pFileCoord
	* @brief EntityFile in CommonFolder
*/	
	FILE *m_pFileCoord;

/**
	* @var m_pFileWrite
	* @brief EntityFile in CommonFolder
*/	
	FILE *m_pFileWrite;

/**
	* @var m_pFileLoad
	* @brief EntityFile in CommonFolder
*/
	FILE *m_pFileLoad;

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

	BOOL m_bIsPolyLoaded;
	BOOL m_bLoadStopFlag;
	int m_iTotalProgress;
	int m_iProgressCount;
public:
	void SetLoadStop(BOOL bStopFlag);
	void SetProgress(int iNum);
	int GetProgressTotal(void);
	int GetProgress(void);
	double Cal_Angle(double cx, double cy, double x, double y);
private:
	int FindLayerNumber(CString strLayerName);
	int m_nInspectionCnt;
	double dInspPosX[4];
	double dInspPosY[4];
public:
	void SortByLayerName(void);
};

