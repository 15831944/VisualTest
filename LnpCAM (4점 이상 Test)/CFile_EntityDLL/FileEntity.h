#include <share.h>
#pragma once

#define _ROW_LINE	0
#define _COL_LINE	1

#define _SECT_EMPTY	-2
#define _SECT_INPUT -1


class AFX_EXT_CLASS CFileEntity :public CObject 
{
public:
	CFileEntity(void);
	~CFileEntity(void);
/**
	* @fn LoadEntityFile(void)
	* @brief Loaded entity file and make block
	* @return None
	* @see InitializeLayerMemory()
	* @see SetBufferEntity()
	* @see CheckMinMax()
	* @see MakeBlock()
	* @see MakeBlockSort()
	* @see SortBlock_StagePos()
	* @see SaveBlockInformation()
*/
	int LoadEntityFile(void);

	int ApplyUserSector(void);
/**
	* @fn GetLayerName(int iLayerNumber)
	* @brief Return the Layer Name
	* @return Layer Name
	* @param [In] iLayerNumber Layer Number
	* @see None
*/
	CString GetLayerName(int iLayerNumber);
/**
	* @fn GetCoordFolder(int iLayerNumber)
	* @brief Return the Coord Folder
	* @return Coord Folder
	* @see None
*/
	CString GetCoordFolder(void);
/**
	* @fn GetLayerCount(void)
	* @brief Return the Layer Count
	* @return Layer Count
	* @see None
*/
	int GetLayerCount(void);
/**
	* @fn GetLayerPointCount(int iLayerNumber)
	* @brief Return the selected layer point count
	* @return point count
	* @param [In] iLayerNumber Layer Number
	* @see None
*/
	int GetLayerPointCount(int iLayerNumber);
/**
	* @fn GetLayerHoleSize(int iLayerNumber)
	* @brief Return the selected layer hole size
	* @return hole size
	* @param [In] iLayerNumber Layer Number
	* @see None
*/

	void InitializeLayerMemory(void);
/**
	* @fn InitializeFileFolder(void)
	* @brief Initialize File Folder
	* @return None
	* @see None
*/


	double GetLayerHoleSize(int iLayerNumber);

	void SetLayerHoleSize(int iLayerNumber, double fHoleSize);

private:
/**
	* @fn GetLineEntity(CString& buf)
	* @brief Loaded a word in file
	* @return result of file load
	* @param [Out] buf Loaded word
	* @see None
*/
	int GetLineEntity(CString& buf);
/**
	* @fn InitializeLayerMemory(void);
	* @brief Initialize all variable
	* @return None
	* @see None
*/

	void InitializeFileFolder(void);
/**
	* @fn SetBufferEntity(void)
	* @brief Loaded entity file and make memory buffer
	* @return result of file load
	* @see GetLineEntity()
*/
	int SetBufferEntity(void);

	int SetBufferOriginEntity(void);
/**
	* @fn CheckMinMax(int iLayerNumber)
	* @brief Check drawing coordnate Min, Max value
	* @return None
	* @param [In] iLayerNumber Layer Number
	* @see CheckMinMaxPoint()
*/
	int CheckMinMax(int iLayerNumber);
/**
	* @fn CheckMinMaxPoint(int iLayerNumber, int iPointNumber)
	* @brief Check drawing coordnate Min, Max value
	* @return None
	* @param [In] iLayerNumber Layer Number
	* @param [In] iPointNumber Point Number
	* @see None
*/
	void CheckMinMaxPoint(int iLayerNumber, int iPointNumber);
/**
	* @fn MakeBlock(int iLayerNumber)
	* @brief Make block and save file
	* @return result of file load
	* @param [In] iLayerNumber Layer Number
	* @see None
*/
	int MakeBlock(int iLayerNumber);
/**
	* @fn MakeBlockSort(int iLayerNumber)
	* @brief Sort by block number
	* @return result of file load
	* @param [In] iLayerNumber Layer Number
	* @see None
*/
	int MakeBlockSort(int iLayerNumber);
/**
	* @fn SavePointData(int iLayerNumber, int iBlockNumber, int iPointCount)
	* @brief Save point Information to file 
	* @return None
	* @param [In] iLayerNumber Layer Number
	* @param [In] iBlockNumber Block Number
	* @param [In] iPointCount Point Count
	* @see SavePointData()
*/
	void SavePointData(int iBlockNumber, double fCX, double fCY);

	void SaveLineData(int iBlockNumber, double fSX, double fSY, double fEX, double fEY);
/**
	* @fn GetLineBlockFile(CString& buf)
	* @brief Loaded a word in file
	* @return result of file load
	* @param [Out] buf Loaded word
	* @see None
*/
	int GetLineBlockFile(CString& buf);
/**
	* @fn SortBlock_StagePos(int iLayerNumber)
	* @brief Sort by block position
	* @return None
	* @param [In] iLayerNumber Layer Number
	* @see Calculate2PointDistance()
*/
	int SortBlock_StagePos(int iLayerNumber);

/**
	* @fn SaveBlockInformation(int iLayerNumber)
	* @brief Save the Block Information
	* @return result of file load
	* @param [In] iLayerNumber Layer Number
	* @see None
*/
	int SaveBlockInformation(int iLayerNumber);

public:

/**
	* @struct POINT_Information
	* @brief Define Point Information
*/
	typedef struct{
		int iBlock;						/**< structure member 01 */
		double fX;						/**< structure member 02 */
		double fY;						/**< structure member 03 */
		BOOL bUsedFlag;					/**< structure member 04 */
	}POINT_Information;

/**
	* @struct Entityi_Information
	* @brief Define Point Information
*/
	typedef struct _Information_Entity{
		int nType;						/*  */
		int nColor;
		int iBlock;						/**< structure member 01 */
		double sX;						/**< structure member 02 */
		double sY;						/**< structure member 03 */
		double eX;	
		double eY;
		double cX;
		double cY;
		double rad;
		double sA;
		double eA;
		double ang;
		BOOL bUsedFlag;					/**< structure member 04 */ //Point : 섹터가 겹칠 때

		int tempSize;
		_Information_Entity *tempLine;
	}Entity_Information;

/**
	* @struct BLOCK_INFO
	* @brief Define Block Information
*/
	typedef struct 
	{ 

		long lBlockNumber;				/**< structure member 01 */
		int lOrderNumber;

		double fLeft;
		double fRight;
		double fTop;
		double fBottom;

		double fX;						/**< structure member 02 */
		double fY;						/**< structure member 03 */
		Entity_Information	*m_pEntity;

	}BLOCK_INFO;

	typedef struct _DATA_LINE
	{ 

		double sx;
		double sy;
		double ex;
		double ey;

	}LINE;

	typedef struct _DATA_SECTOR_LINE
	{ 
		int RowCnt;
		int ColCnt;

		LINE *Row;
		LINE *Col;

	}SECTOR_LINE;
	

	BLOCK_INFO* m_pBlockPos;			//Block Info about All Layers.
	int m_iBlockCountX;				
	int m_iBlockCountY;				
	int m_iMaxBlockNumber;			

	SECTOR_LINE m_SectLine;

	//int m_iRealBlockCount;		

/**
	* @struct LAYER
	* @brief Define Layer Information
*/
	typedef struct 
	{
		int m_nOptimizeType;

		CString strLayerName;			/**< structure member 01 */
		//int i_LayerColor;
		int iEntityCount;				/**< structure member 02 */
		int iOriginCount;
		int m_iBlockCount;				/**< structure member 03 */
		int iRealBlockCount;


		Entity_Information* pOriginEntity; //블록 나누기 전 Entity
		Entity_Information* pEntity;	   //블록 나눈 후 Entity
		//BLOCK_INFO* pBlockPos;			/**< structure member 05 */
		BLOCK_INFO* pCutBlock;			/**< structure member 05 */

		int * pInBlockEntityCount;		/**< structure member 06 */
		int * pBlockOrder;				/**< structure member 07 */

		double fHoleSize;				/**< structure member 08 */
		double fMinX;					/**< structure member 09 */
		double fMinY;					/**< structure member 10 */
		double fMaxX;					/**< structure member 11 */
		double fMaxY;					/**< structure member 12 */
		//int m_iBlockCountX;				/**< structure member 13 */
		//int m_iBlockCountY;				/**< structure member 14 */
		//int m_iMaxBlockNumber;			/**< structure member 15 */
		//int m_iRealBlockCount;			/**< structure member 16 */

	}LAYER;
/**
	* @var m_pLayer
	* @brief Define variable of LAYER
*/
	LAYER* m_pLayer; 


/**
	* @var EntityFileOut
	* @brief File for read entity file
*/
	FILE* EntityFileOut;
/**
	* @var BlockFileIn
	* @brief File for save Block Information file
*/
	FILE* BlockFileIn;
	FILE* CuttingFileIn;
	FILE* JobFileOut;
/**
	* @var BlockFileOut
	* @brief File for read Block Information file
*/
	FILE* BlockFileOut;
/**
	* @var m_strCoordFolder
	* @brief Define Coord Folder
*/
	CString m_strCoordFolder; 

/**
	* @var m_strCommonFolder
	* @brief Define Common Folder
*/
	CString  m_strCommonFolder;

/**
	* @var m_strCommandLine
	* @brief Define Command Line
*/
	CString m_strCommandLine;

/**
	* @var m_iLayerCount
	* @brief Define Layer Count
*/
	int m_iLayerCount;
/**
	* @var m_iBlockSize
	* @brief Define Block Size
*/
	int m_iBlockSize;
/**
	* @var m_lFilePointCount
	* @brief Define All Point Count
*/
	long m_lFilePointCount;

	double m_fOverlap;

	double m_fTotalMinX;
	double m_fTotalMinY;
	double m_fTotalMaxX;
	double m_fTotalMaxY;

	BOOL m_bRemoveBlank;
	BOOL m_bCenterAlign;
	BOOL m_bOptimization;
	BOOL m_bMirror;
	BOOL m_bReverse;
	BOOL m_bLoadStopFlag;
	int m_iTotalProgress;
	int m_iProgressCount;
	CString m_strProgressMsg;

	CString m_strBaseFile;
public:
	void SetLoadStop(BOOL bStopFlag);
	void SetProgress(int iNum);
	void SetProgressMsg(CString str);
	int GetProgressTotal(void);
	int GetProgress(void);
	CString GetProgressMsg(void);
	void SetOptimization(BOOL bFlag);
	void SetCenterAlign(BOOL bFlag);
	void SetRemoveBlank(BOOL bFlag);
	void SetMirror(BOOL bFlag);
	void SetReverse(BOOL bFlag);

private:
	int LoadBlockData(int iLayerNumber);
	int DoReverse(int iLayerNumber);
	int DoMirror(int iLayerNumber, int nVersion);
	int GetLineCutting(CString& buf);
	void SetTotalProgress(void);
protected:
	int m_nVersion;
public:
	int DoOptimization(int iLayerNumber);
	int SaveJobData(CString strFilepath);
	void SetBlockSize(int BlockSize);
	void SetEntityVersion(int nVersion);
	int m_nInspectionCnt;
	double m_dInspectionPosX[4];
	double m_dInspectionPosY[4];
	int GetInspectionCnt(void);
	double GetInspectionPos(int nIndex, CString strAxis);

	void CheckMinMaxTotalPoint(double fcX, double fcY);
	void CheckMinMaxTotalCircle(double fcX, double fcY, double fR);
	void CheckMinMaxTotalArc(double fcX, double fcY, double fR, double sA, double eA);
	void CheckMinMaxTotalLine(double fsX, double fsY, double feX, double feY);

	int	MakeBlockAllLayer(void);
	
	int DeleteSectorLineMem();
	int InitSectorLine();

	int ModifySectorLineCoord(int rowNum, int ColNum, double rowCoordChange, double colCoordChange);
	
	int AddSectorLine(int LineType);
	int DeleteSectorLine(int LineType);

	int ExtensionSectorLine(int LineType, BOOL bFlag);
	BOOL CheckSectorArea(void);
	void SetSectorCenterAlign(void);
	
	double m_dSectorTotalMoveX;
	double m_dSectorTotalMoveY;
	void   GetSectorTotalMove(double *dX, double *dY);

	//Ret 0: Find Sector = Stop Loop, Ret 1: Find Failed
	int ExploidPoint(int nLayerNum, int nEntityNum);	
	int ExploidLine(int nLayerNum, int nEntityNum);
	int ExploidTemp(int nLayerNum, int nEntityNum, int nTemp);

	void InitSectorOrder(int iLayerNum);
	void InputSectorOrder(void);

	void ModifyORGCoord(double fMoveX, double fMoveY);

	int m_nSelLayer;
	void SelectLayer(int iLayerNumber);
	BOOL m_bSectorApply;

public:
	void ArcToLine(int nLayer, int nEntity);
	void EllipseToLine(int nLayer, int nEntity);

	/**
	* @fn Calculate2PointDistance(double fx1, double fy1, double fx2, double fY2)
	* @brief Distance measurement between two points
	* @return Distance between two points
	* @param [In] fx1 point 1 X coordnate 
	* @param [In] fy1 point 1 Y coordnate
	* @param [In] fx2 point 2 X coordnate
	* @param [In] fY2 point 2 Y coordnate
	* @see None
*/
	double Calculate2PointDistance(double fx1, double fy1, double fx2, double fY2);
/**
	* @fn DoRound(double i_dNumber)
	* @brief round the number
	* @return rounded number
	* @param [in] i_dNumber Original number
	* @see None
*/
	double CalculateAngle2Point(double x1, double y1, double x2, double y2);

	int SCAN_FindIntersection(double sx, double sy, double ex, double ey, double minx, double maxx, 
		double miny, double maxy, double *x, double *y, int dummy);

	double DoRound(double dNumber);

	void AllocLayerMem(int nSize);
	void AllocLayerEntityMem(int nLayer, int nSize);
	void AllocSectLineMem(int nRowSize, int nColSize);
	void AllocCutBlock(int nLayer, int nSize);
	void AllocCutBlockEntity(int nLayer, int nBlock, int nSize);

	int Change_um(double val);
	double Change_float(int num);

	bool m_b3StepMode;
	int m_nLayerBufferSize;

	BOOL StartOptimize(void);

	int nNum;

	clock_t before;
	clock_t after;

	int m_nXCount;
	int m_nYCount;

	double m_fIdealPointX[12];
	double m_fIdealPointY[12];

	double m_fRealPointX[12];
	double m_fRealPointY[12];

	double dHomography[8];

	void ApplyInspectResult(void);
};

static CWinThread** hProcessOptimizeTH;
UINT ThreadOptimize(LPVOID pParam);




