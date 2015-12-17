#pragma once

#define	_ENTITY_POINT	1
#define	_ENTITY_DRILL	1
#define	_ENTITY_LINE	2
#define	_ENTITY_RECT	3
#define	_ENTITY_CIRCLE	4
#define	_ENTITY_ARC		5
#define _ENTITY_ELLIPSE 6

class AFX_EXT_CLASS CFileLpd : public CObject 
{
public:
	CFileLpd(void);
	~CFileLpd(void);

	BOOL LoadLpdFile(CString strPath);

	typedef struct _DATA_LINE{

		double sX;
		double eX;
		double sY;
		double eY;

	}LINE_DATA;

	typedef struct _DATA_ENTITY{

		int nType;						
		int nColor;
		double sX;						
		double sY;						
		double eX;	
		double eY;
		double cX;
		double cY;
		double rad;
		double sA;
		double eA;
		double ang;

	}ENTITY_DATA;

	typedef struct _DATA_BLOCK 
	{ 
		double fX;						/**< structure member 02 */
		double fY;						/**< structure member 03 */

		int nEntityCnt;
		ENTITY_DATA	*pEntity;

	}BLOCK_DATA;

	typedef struct _DATA_LAYER{

		CString strLayerName;

		double dHoleSize;

		int		nSectorCount;

		int * pBlockOrder;

		int nEntityCount;

		BLOCK_DATA *pBlock;


	}LAYER_DATA;
	
	int m_nLPDVersion;

	int	   m_nInspectionCount;
	double m_dInspectionPosX[4];
	double m_dInspectionPosY[4];

	double m_dSectorSize;

	int m_nEntityCount;

	int m_nBlockCntX;
	int m_nBlockCntY;

	int			m_nLayerCnt;
	LAYER_DATA *m_pLayer;

	int		m_nBlockLineCntX;
	LINE_DATA	*m_SectLineX;

	int		m_nBlockLineCntY;
	LINE_DATA	*m_SectLineY;

	CString m_strFilePath;

	double  m_dOverlapSize;		
	double  m_dOffsetX;
	double  m_dOffsetY;
};