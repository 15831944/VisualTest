#pragma once
#include "afxwin.h"
#include "FileEntity.h"

// GDIPLUS library
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

//#include <gl\gl.h>
//#include <gl\GLU.h>
#include <gl\glut.h>

#define IDM_ORGVIEW                     32787
#define IDM_ABSVIEW                     32788
#define IDM_ABSMOVE                     32789
#define IDM_SECTOR                      32790
#define IDM_CHANGE_SECTOR               32791

#define IDM_VIEWARROW		            32792


class AFX_EXT_CLASS CFileEntityViewer :	public CWnd
{
public:
	CFileEntityViewer(void);
	~CFileEntityViewer(void);

	CFileEntity *pData;

	void CreateView(CFileEntity *pFileEntity);

protected:
	// Generated message map functions
	//{{AFX_MSG(CModule_FileViewer)

	HDC m_hDC;
	HGLRC m_hRC;

	BOOL **m_bUse;
	int m_nUnit;
	int m_nCurUnit;
	int m_nWidth;
	int m_nHeight;

	afx_msg void OnPaint();
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnViewArrow();
	afx_msg void OnAbsmove();
	afx_msg void OnAbsview();
	afx_msg void OnOrgview();
	afx_msg void OnSector();
	afx_msg void OnChangeSector();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

	//Struct

	typedef struct _DATA_SECTOR_POINT
	{
		double posX,posY;
		int Row_Line;
		int Col_Line;
		BOOL Mouse_Flag;
	}SECTOR_POINT;


	typedef struct _DATA_SECTOR_LINE
	{
		int sx,sy,ex,ey;
	}SECTOR_LINE;


	typedef struct _DATA_LINE
	{
		CString m_layer;
		int line_flag;
		double sx,sy,ex,ey;
		double cx,cy,r,sa,ea;
		int l_color;
	}LINE;

	typedef struct _DATA_ENTITY
	{
		int nType;		//Arc, Pen..

		double dSX, dSY, dEX, dEY;

		double dCX, dCY, dR, dSA, dEA;

		int nColor;

	}ENTITY_DATA;

	typedef struct _DATA_LAYER
	{
		CString strName;
		int		nType;		//Rout, Drill
		ENTITY_DATA	*pEntity;
		int		nEntityCnt;

	}LAYER_DATA;


	typedef struct _DATA_COLOR
	{
		int C_R;
		int C_G;
		int	C_B;
	}COLOR_value;


	//=================================================================================================

	/* Util */

	int Change_int(double val);
	int Change_um(double val);
	double Change_float(int num);
	double Round(double number);

	void MakeFolders(void);


	//=================================================================================================

	/* Calculate */

	void Cal_Sector_Point();
	int Layer_Color(int color);


	//=================================================================================================

	/* UserSector */


	//void Init_Sector_Line();
	
	void Insert_Col_Line();						
	void Insert_Row_Line();						
	void Delete_Sector_Line(int line);


	//===================================================================================================

	/* Loading Entity & Buffer */

	void SetSectorSize(int number);
	void SetFileLoad(BOOL bFlag);

	void Initialize();

	void SetLayerViewer(int nLayerNum, BOOL bFlag);	


	//===================================================================================================

	/* Drawing */

	BOOL InitViewer(CWnd* pParent);

	void View_Draw_Line();
	void View_Draw_SectorLine(Graphics *pDC);
	void View_Draw_SectorOrder(Graphics *pDC);
	void View_Draw_Sector(Graphics *pDC);
	void View_Draw_AbsMove(Graphics *pDC);
	void View_Draw_AbsView(Graphics *pDC);
	void View_Draw_Ruler(Graphics *pDC);

	void MouseWheel(UINT nFlags, short zDelta, CPoint pt);

	void Close_FileView();

	//====================================================================================================

	/* reference */

	void SetClickMove(BOOL flag);

	void SetViewArrow(void);
	void SetViewAbs(void);
	void SetAbsMove(void);
	void SetViewSector(void);
	void SetModifySector(void);
	void SetViewORG(void);
	void SetModifyORG(double fMoveX, double fMoveY);
	void SetVIewOrder(BOOL bFlag);
	void SetModifyOrder(BOOL bFlag);
	void SetViewPattern(void);
	void SetModifyPattern(void);

	void GetCoord(float* fCoordX, float* fCoordY);

	void InitInputNumMEM();


	//===================================================================================================


	//====================================================================================================

	/* Sector */

	SECTOR_POINT * m_Sector_Point;
	int m_Sector_Point_Cnt;

// 	SECTOR_LINE * m_Sector_Line_Row;
// 	SECTOR_LINE * m_Sector_Line_Col;

// 	int m_Sector_Line_Row_Cnt;
// 	int m_Sector_Line_Col_Cnt;

	int Sel_MoveLine;							// 섹터 변경시 이동할 라인

	int FindSectorNum(double fX, double fY);
	
	int GetOrderModifyState(int nLayerNum);		// 1: DONE, 0: NOT DONE

	//int m_nInputNum;

	//====================================================================================================

	/* Folders */

	CString m_strDxfFolder;
	CString m_strBlockFolder;
	CString m_strCommonFolder;
	CString m_strEntityFolder;
	CString m_strCoordFolder;


	//====================================================================================================

	/* Flags */

	BOOL bClickMove;

	BOOL m_bApply_flag;							// 파일적용 여부 체크 ( 클릭 무브 안전 위함 )
	BOOL m_bFile_LoadIs;						

	BOOL bViewArrow;
	BOOL bool_orgview;
	BOOL bool_absview;
	BOOL bool_absmove;
	BOOL bool_absmove2;
	BOOL bool_sector;							//SectorView
	BOOL bool_change_sector;					//SectorModify

	BOOL m_bOrderView;
	BOOL m_bOrderModify;

	BOOL *m_bViewLayer;
	int *m_nInputNum;

	BOOL ClickFlag;


	//====================================================================================================

	/* File info */

	int SCAN_Length;

	int m_SectorSize;

	//====================================================================================================

	/* Coord */

	//For Abs Move
	CString m_viewer_abs_x, m_viewer_abs_y;
	double abs_move_posx, abs_move_posy;
	double Posx, Posy, Posz;

	double m_dFileStartX;						// Click Move시 이동 거리 계산. 외부 모듈서 설정하는 듯. DANGER!!!
	double m_dFileStartY;
	double m_dCurrentX;
	double m_dCurrentY;

	double Mouse_MoveX;							// 현재 Cursor 좌표
	double Mouse_MoveY;

	double Mini_Rectx, Mini_Recty;							//클릭 무브 사각형 크기

	double m_MaxX, m_MaxY, m_MinX, m_MinY;					//도면의 최소, 최대 좌표


	//====================================================================================================

	CMenu menu;

	int View_X, View_Y;

	int m_msx, m_msy; 

	int m_mlx, m_mly;

	int ruler_gap; //자 간격

	int buffer_count;

	double m_right; //좌우 0,0의 위치
	double m_updown; //상하

	double m_zoom; //확대,축소를 위한 변수
	double m_dxfzoom; //뷰어에 처음 보일때 확대,축소를 위한 변수

	COLOR_value m_Layer_Color;
	COLOR_value m_PrevColor;

	void SetSectorMove(double dMoveX, double dMoveY);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	// OpenGL
	void BuildFont(void);
	void RemoveFont(void);

	GLuint base;

	void DrawText(int WinPosX, int WinPosY, CString strMsg, void* font, double Color0, double Color1, double Color2);
};

