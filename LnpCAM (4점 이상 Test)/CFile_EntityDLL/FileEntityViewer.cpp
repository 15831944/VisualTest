#include "StdAfx.h"
#include "FileEntityViewer.h"
#include <cmath>

#define CW -1 //ARC 해석시 원호의 방향 비트
#define CCW 1 //ARC 해석시 원호의 방향 비트
#define PHI 3.141592653589793238462643383279 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CFileEntityViewer::CFileEntityViewer(void)
{
	MakeFolders();

	ruler_gap=5;

	bool_orgview=FALSE;
	bool_absview=FALSE;
	bool_absmove=FALSE;
	bool_absmove2=FALSE;
	bool_sector=FALSE;

	m_bOrderView = FALSE;
	m_bOrderModify = FALSE;

	m_SectorSize = 10;

	Sel_MoveLine = -1;
	m_Sector_Point		= NULL;
//	m_Sector_Line_Row	= NULL;
//	m_Sector_Line_Col	= NULL;
	m_bFile_LoadIs		= FALSE;

	bClickMove			= FALSE;

	m_PrevColor.C_B		= 0;
	m_PrevColor.C_R		= 0;
	m_PrevColor.C_G		= 0;

	m_bViewLayer = NULL;
	m_nInputNum = NULL;
	m_bUse			=	NULL;

	bViewArrow = FALSE;

	bool_change_sector = FALSE;
}


CFileEntityViewer::~CFileEntityViewer(void)
{
	Close_FileView();

	if(m_bUse != NULL)
	{
		for(int i=0; i<m_nWidth; i++)
		{
			delete[] m_bUse[i];
		}
		delete[] m_bUse;
	}

	if(m_Sector_Point!=NULL)	{	delete[] m_Sector_Point;}
	if(m_bViewLayer != NULL)	{	delete[] m_bViewLayer;}
	if(m_nInputNum != NULL)		{	delete[] m_nInputNum;}
}


void CFileEntityViewer::CreateView(CFileEntity *pFileEntity)
{
	pData = (CFileEntity *) pFileEntity;
}

BEGIN_MESSAGE_MAP(CFileEntityViewer, CWnd)
	ON_WM_PAINT()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(IDM_ABSMOVE, OnAbsmove)
	ON_COMMAND(IDM_ABSVIEW, OnAbsview)
	ON_COMMAND(IDM_ORGVIEW, OnOrgview)
	ON_COMMAND(IDM_SECTOR, OnSector)
	ON_COMMAND(IDM_CHANGE_SECTOR, OnChangeSector)
	ON_COMMAND(IDM_VIEWARROW, OnViewArrow)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_WM_TIMER()
	ON_WM_CREATE()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEHWHEEL()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


void CFileEntityViewer::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	View_Draw_Line();
}

void CFileEntityViewer::View_Draw_Line()
{
	/* OpenGL 이용*/

	glShadeModel(GL_SMOOTH);
	glClearColor(0, 0.0f, 0.0f, 1.0f);
	//glClearDepth(1.0f);
	//glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	wglMakeCurrent(m_hDC, m_hRC);
	
	glLineWidth(1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();

	


	double a1,a2,a3,a4;

	//인식 마크 Ideal

	glColor3f(200/255.0f, 50/255.0f, 200/255.0f);

	for(int i=0; i<4; i++)
	{
		a1=pData->m_fIdealPointX[i];
		a2=pData->m_fIdealPointY[i];

		a1=a1*m_zoom*m_dxfzoom+m_right;
		a2=a2*m_zoom*m_dxfzoom+m_updown;
		a3=2;
		
		glBegin(GL_LINE_STRIP);
		glVertex2f(a1-10 , a2);	
		glVertex2f(a1+10 , a2);	
		glEnd();

		glBegin(GL_LINE_STRIP);
		glVertex2f(a1 , a2-10);	
		glVertex2f(a1 , a2+10);	
		glEnd();
	}

	//인식 마크 Real

	glColor3f(50/255.0f, 200/255.0f, 200/255.0f);

	for(int i=0; i<4; i++)
	{
		a1=pData->m_fRealPointX[i];
		a2=pData->m_fRealPointY[i];

		a1=a1*m_zoom*m_dxfzoom+m_right;
		a2=a2*m_zoom*m_dxfzoom+m_updown;
		a3=10;

		glBegin(GL_LINE_LOOP);
		for (int nC=0; nC<360; nC++)
		{
			float degInRad = nC*3.14159/180;
			glVertex2f(a1 + cos(degInRad)*a3,a2 + sin(degInRad)*a3);
		}
		glEnd();
	}
	
	int lr=255,lg=255,lb=255;

	for(int nLayerNum=0; nLayerNum<pData->m_iLayerCount; nLayerNum++)
	{
		if(m_bViewLayer[nLayerNum] == FALSE)
			continue;
		
		Layer_Color(pData->m_pLayer[nLayerNum].pOriginEntity[0].nColor);
		lr=m_Layer_Color.C_R;
		lg=m_Layer_Color.C_G;
		lb=m_Layer_Color.C_B;

		//penArc.SetColor(Color(lr,lg,lb));
		glColor3f(lr/255.0f, lg/255.0f, lb/255.0f);

		for( int i=0;i<pData->m_pLayer[nLayerNum].iOriginCount;i++)
		{
			BOOL bContinue = FALSE;

			switch(pData->m_pLayer[nLayerNum].pOriginEntity[i].nType)
			{
			case _ENTITY_POINT: //point
				{
					a1=pData->m_pLayer[nLayerNum].pOriginEntity[i].cX;
					a2=pData->m_pLayer[nLayerNum].pOriginEntity[i].cY;
					a3=pData->m_pLayer[nLayerNum].fHoleSize/2;
					a1=a1*m_zoom*m_dxfzoom+m_right;
					a2=a2*m_zoom*m_dxfzoom+m_updown;
					a3=a3*m_zoom*m_dxfzoom;

					if(a3 < .7)
						a3 = .7;
					
					glBegin(GL_POLYGON);
					for (int nC=0; nC<360; nC++)
					{
						float degInRad = nC*3.14159/180;
						glVertex2f(a1 + cos(degInRad)*a3,a2 + sin(degInRad)*a3);
					}
					glEnd();
				}
				break;
			case _ENTITY_LINE: //line
				{

					a1=pData->m_pLayer[nLayerNum].pOriginEntity[i].sX;
					a2=pData->m_pLayer[nLayerNum].pOriginEntity[i].sY;
					a3=pData->m_pLayer[nLayerNum].pOriginEntity[i].eX;
					a4=pData->m_pLayer[nLayerNum].pOriginEntity[i].eY;

					a1=a1*m_zoom*m_dxfzoom+m_right;
					a2=a2*m_zoom*m_dxfzoom+m_updown;
					a3=a3*m_zoom*m_dxfzoom+m_right;
					a4=a4*m_zoom*m_dxfzoom+m_updown;

					glBegin(GL_LINE_STRIP);
					glVertex2f(a1 , a2);	
					glVertex2f(a3 , a4);	
					glEnd();

					//double dRot = pData->CalculateAngle2Point( a1, a2, a3, a4);

					if(bViewArrow)
					{
						double dRot = atan2((a4 - a2),(a3 - a1));
						double dArrowSize = 0.15 * m_zoom * m_dxfzoom;
						if(dArrowSize > 5) dArrowSize = 5;
						else if (dArrowSize < 2) dArrowSize = 2;

						double dX1 = (-dArrowSize*2) * cos(dRot) - (dArrowSize)* sin(dRot);
						double dY1 = (-dArrowSize*2) * sin(dRot) + (dArrowSize)* cos(dRot);

						double dX2 = (-dArrowSize*2) * cos(dRot) - (-dArrowSize)* sin(dRot);
						double dY2 = (-dArrowSize*2) * sin(dRot) + (-dArrowSize)* cos(dRot);

						glBegin(GL_LINE_STRIP);
						glVertex2f(a3+dX1 , a4+dY1);
						glVertex2f(a3	,   a4	);
						glVertex2f(a3+dX2 , a4+dY2);
						glEnd();					
					}
				}
				break;
				case _ENTITY_CIRCLE: //circle
				a1=pData->m_pLayer[nLayerNum].pOriginEntity[i].cX;
				a2=pData->m_pLayer[nLayerNum].pOriginEntity[i].cY;
				a3=pData->m_pLayer[nLayerNum].pOriginEntity[i].rad;

				a1=a1*m_zoom*m_dxfzoom+m_right;
				a2=a2*m_zoom*m_dxfzoom+m_updown;
				a3=a3*m_zoom*m_dxfzoom;
			
				glBegin(GL_LINE_LOOP);
				for (int nC=0; nC<360; nC++)
				{
					float degInRad = nC*3.14159/180;
					glVertex2f(a1 + cos(degInRad)*a3,a2 + sin(degInRad)*a3);
				}
				glEnd();
				break;
			case _ENTITY_ARC: //arc
				{
					a1=pData->m_pLayer[nLayerNum].pOriginEntity[i].cX;
					a2=pData->m_pLayer[nLayerNum].pOriginEntity[i].cY;
					a3=pData->m_pLayer[nLayerNum].pOriginEntity[i].rad;
					a4=pData->m_pLayer[nLayerNum].pOriginEntity[i].sA;
					double a5=pData->m_pLayer[nLayerNum].pOriginEntity[i].eA;

					double sX, sY;
					double eX, eY;

					sX = pData->m_pLayer[nLayerNum].pOriginEntity[i].sX;
					eX = pData->m_pLayer[nLayerNum].pOriginEntity[i].eX;
					sY = pData->m_pLayer[nLayerNum].pOriginEntity[i].sY;
					eY = pData->m_pLayer[nLayerNum].pOriginEntity[i].eY;

					sX=sX*m_zoom*m_dxfzoom+m_right;
					sY=sY*m_zoom*m_dxfzoom+m_updown;
					eX=eX*m_zoom*m_dxfzoom+m_right;
					eY=eY*m_zoom*m_dxfzoom+m_updown;

					a1=a1*m_zoom*m_dxfzoom+m_right;
					a2=a2*m_zoom*m_dxfzoom+m_updown;
					a3=a3*m_zoom*m_dxfzoom;

					//Viewer에서는 Y축이 반전되어 각도를 변환시켜줘야함.<360 - ANGLE>
					//a4 = 360 - pData->m_pLayer[nLayerNum].pOriginEntity[i].sA;
					//a5 = 360 - pData->m_pLayer[nLayerNum].pOriginEntity[i].eA;
					
					//REAL a[] = {(float)(a1-a3), (float)(a2-a3), (float)(2*a3), (float)(2*a3), (float)a4, (float)(a5-a4)};
					
					
					float degInRadSa = a4*3.14159/180;
					float degInRadEa = a5*3.14159/180;

					int segments = 40;
					float d_angle = degInRadEa-degInRadSa;
					int real_segments = int(fabsf(d_angle) / (2 * 3.141592) * (float)segments) + 1;
					//int real_segments = abs(a5-a4)
	
					float theta = d_angle / float(real_segments);
					float tangetial_factor = tanf(theta);
					float radial_factor = 1 - cosf(theta);
	
					float x = a1 + a3 * cosf(degInRadSa);
					float y = a2 + a3 * sinf(degInRadSa);

					glBegin(GL_LINE_STRIP);
					for(int ii = 0; ii < real_segments + 1; ii++)
					{
						glVertex2f(x, y);
		
						float tx = -(y - a2);
						float ty = x - a1;
		
						x += tx * tangetial_factor;
						y += ty * tangetial_factor;
		
						float rx = a1 - x;
						float ry = a2 - y;
		
						x += rx * radial_factor;
						y += ry * radial_factor;
					}
					glEnd();
					
				}
				break;
				
			case _ENTITY_ELLIPSE: //arc
				{
					/*
					a1=pData->m_pLayer[nLayerNum].pOriginEntity[i].sX;
					a2=pData->m_pLayer[nLayerNum].pOriginEntity[i].sY;
					a3=pData->m_pLayer[nLayerNum].pOriginEntity[i].rad;
					a4=pData->m_pLayer[nLayerNum].pOriginEntity[i].sA;
					a5=pData->m_pLayer[nLayerNum].pOriginEntity[i].eA;
					a1=a1*m_zoom*m_dxfzoom+m_right;
					a2=View_Y-a2*m_zoom*m_dxfzoom+m_updown;
					a3=a3*m_zoom*m_dxfzoom;
					a4=360-a4;
					a5=360-a5;

					double sx, sy, ex, ey, radAng, temp_x,temp_y,temp_sa,temp_ea, ratio, cx, cy, radRotation;
					bool bStop = false;
					double add_i;
					double A, B;

					
					temp_sa = pData->m_pLayer[nLayerNum].pOriginEntity[i].sA;
					temp_ea = pData->m_pLayer[nLayerNum].pOriginEntity[i].eA;

					ratio = pData->m_pLayer[nLayerNum].pOriginEntity[i].rad;

					sx = pData->m_pLayer[nLayerNum].pOriginEntity[i].sX;
					sy = pData->m_pLayer[nLayerNum].pOriginEntity[i].sY;

					cx = pData->m_pLayer[nLayerNum].pOriginEntity[i].cX;
					cy = pData->m_pLayer[nLayerNum].pOriginEntity[i].cY;

					A = pData->Calculate2PointDistance(sx, sy, cx, cy); //장축
					B = A*ratio;								//단축

					radRotation = atan2(sy - cy, sx - cx);// * 180. / PHI;
					double zz = radRotation *180 / PHI;

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

					for(;;)
					{
						temp_sa += add_i;

						if(pData->DoRound(temp_sa)>=pData->DoRound(temp_ea)) 
						{
							temp_sa = temp_ea;
							bStop = true;
						}

						radAng = (temp_sa) * PHI / 180.;

						temp_x = cx + A * cos(radAng)*cos(radRotation) - B * sin(radAng) * sin(radRotation);
						temp_y = cy + A * cos(radAng)*sin(radRotation) + B * sin(radAng) * cos(radRotation);

						ex = temp_x;
						ey = temp_y;

						MemDC.DrawLine( &penArc, float(temp_x * m_zoom*m_dxfzoom+m_right), float( View_Y - temp_y * m_zoom*m_dxfzoom+m_updown)
							, float(ex * m_zoom*m_dxfzoom+m_right), float(View_Y - ey * m_zoom*m_dxfzoom+m_updown) );

						sx = ex;
						sy = ey;

						if(bStop) break;
						
					}*/
				}

				break;
			}
		}
	}

	if(bool_sector==TRUE)
		View_Draw_Sector(NULL);

	if(m_bOrderView == TRUE){
		View_Draw_SectorOrder(NULL);
	}
	if(bool_absview==TRUE)
		View_Draw_AbsView(NULL);

	if(bool_change_sector==TRUE)
	{
		View_Draw_SectorLine(NULL);
	}

	if(bool_orgview==TRUE)
	{
		glLineWidth(1.4f);

		CString str;
		str = L"X";

		float fX = (float)m_right;
		float fY = (float)m_updown;

		glColor3f(1.0f, 1.0f, 1.0f);

		glBegin(GL_LINE_STRIP);
		glVertex2f(fX-7 , fY);	
		glVertex2f(fX+100 , fY);
		glVertex2f(fX+80 , fY+5);
		glVertex2f(fX+80 , fY-5);
		glVertex2f(fX+100 , fY);
		glEnd();

		glBegin(GL_LINE_STRIP);
		glVertex2f(fX, fY-7);	
		glVertex2f(fX, fY+100);
		glVertex2f(fX+5, fY+80);
		glVertex2f(fX-5, fY+80);
		glVertex2f(fX, fY+100);
		glEnd();

		glBegin(GL_LINE_STRIP);
		glVertex2f(fX-7, fY-7);	
		glVertex2f(fX-7, fY+7);
		glVertex2f(fX+7, fY+7);	
		glVertex2f(fX+7, fY-7);
		glVertex2f(fX-7, fY-7);
		glEnd();

		DrawText(fX+90, fY-20, L"X", GLUT_BITMAP_9_BY_15, 1, 1, 1);
		DrawText(fX-20, fY+90, L"Y", GLUT_BITMAP_9_BY_15, 1, 1, 1);
	}
	
	SwapBuffers(m_hDC);
	wglMakeCurrent(m_hDC, NULL);

	return;

	/* GdiPlus 이용 */
	/*

	if(m_bFile_LoadIs == FALSE)
		return;

	CClientDC dc(this); // device context for painting
	
	CRect rt;
	GetClientRect(rt);

	Gdiplus::Bitmap bitmap(rt.Width(),rt.Height());
	Gdiplus::Graphics gTools(dc);
	Gdiplus::Graphics MemDC(&bitmap);

	GraphicsPath path, path2;
	Point pts[] = { Point(0,0), Point(3,-5),Point(0,0),  Point(-3,-5)};
	path.AddPolygon(pts,4);
	path2.AddEllipse(-3,-3,6,6);

	CustomLineCap custCap(NULL, &path);
	custCap.SetWidthScale(1);
	custCap.SetBaseInset(0);

	CustomLineCap custCap2(NULL, &path2);
	custCap.SetWidthScale(1);
	custCap.SetBaseInset(0);
	
	Gdiplus::SolidBrush blackBrush( Gdiplus::Color(0, 0, 0));
	Gdiplus::SolidBrush whiteBrush( Gdiplus::Color(255, 255, 255));
	MemDC.FillRectangle(&blackBrush, 0, 0, rt.Width(), rt.Height());

	Gdiplus::Font fntGulim(L"굴림체", 15, FontStyleRegular, UnitPixel);
	
	//CRect rect;
	double a1,a2,a3,a4,a5;
	int lr=255,lg=255,lb=255;

	Pen penLimit( Color (0, 255, 0), 2 );
	Pen penArc( Color (255, 0, 0), 1 );
	Pen penText( Color (255, 255, 255), 1 );

	if(bool_absview==TRUE)
		penArc.SetCustomEndCap(&custCap);
	
	m_nUnit = 1;
	double dZoomFactor = m_dxfzoom * m_zoom;

	if(dZoomFactor < 4)
		m_nUnit = 2;

	for(int width=0; width<m_nWidth;width++)
	{
		memset(m_bUse[width], FALSE, sizeof(m_bUse[width]) * m_nHeight);
	} 

	if(bool_orgview==TRUE)
	{
		CString str;
		str = L"X";

		float fX = (float)m_right;
		float fY = (float)(View_Y + m_updown);

		MemDC.DrawLine(&penText ,fX-10, fY, fX+100, fY);
		MemDC.DrawLine(&penText ,fX, fY+10, fX, fY-100);

		MemDC.DrawString(str, str.GetLength(), &fntGulim, PointF(fX+90,fY), &whiteBrush );
		MemDC.DrawString(L"Y", str.GetLength(), &fntGulim, PointF(fX-10,fY-90), &whiteBrush );
	}

	
	CPoint mini1,mini2; //파일뷰 화면에서의 비전화면 부분 
	if(bool_absmove==TRUE)
		View_Draw_AbsMove(&MemDC);


	for(int nLayerNum=0; nLayerNum<pData->m_iLayerCount; nLayerNum++)
	{
		if(m_bViewLayer[nLayerNum] == FALSE)
			continue;
		
		Layer_Color(pData->m_pLayer[nLayerNum].pOriginEntity[0].nColor);
		lr=m_Layer_Color.C_R;
		lg=m_Layer_Color.C_G;
		lb=m_Layer_Color.C_B;

		penArc.SetColor(Color(lr,lg,lb));

		for( int i=0;i<pData->m_pLayer[nLayerNum].iOriginCount;i++)
		{
			BOOL bContinue = FALSE;

			switch(pData->m_pLayer[nLayerNum].pOriginEntity[i].nType)
			{
			case _ENTITY_POINT: //point
				a1=pData->m_pLayer[nLayerNum].pOriginEntity[i].cX;
				a2=pData->m_pLayer[nLayerNum].pOriginEntity[i].cY;
				a3=pData->m_pLayer[nLayerNum].fHoleSize/2;
				a1=a1*m_zoom*m_dxfzoom+m_right;
				a2=View_Y-a2*m_zoom*m_dxfzoom+m_updown;
				a3=a3*m_zoom*m_dxfzoom;

				if( a3 < .6)
					a3 = .6;
				MemDC.FillPie(&whiteBrush, float(a1), float(a2), float(a3*2), float(a3*2), 0, 360);


// 				if(a3 > 1){
// 					if( ( Change_int(a1+a3) < 0		||		Change_int(a1+a3) > View_X ) &&
// 						( Change_int(a1-a3) < 0		||		Change_int(a1-a3) > View_X ) &&	
// 						( Change_int(a2+a3) < 0		||		Change_int(a2+a3) > View_Y ) &&
// 						( Change_int(a2-a3) < 0		||		Change_int(a2-a3) > View_Y ) )
// 						break;
// 
// 						MemDC.FillPie(&whiteBrush, float(a1), float(a2), float(a3*2), float(a3*2), 0, 360);	
// 				}
// 				else
// 				{
// 					if( ( Change_int(a1) < 0		||		Change_int(a1) > View_X	) ||	
// 						( Change_int(a2) < 0		||		Change_int(a2) > View_Y	) )
// 						break;
// 				}
				break;
			case _ENTITY_LINE: //line
				{

					a1=pData->m_pLayer[nLayerNum].pOriginEntity[i].sX;
					a2=pData->m_pLayer[nLayerNum].pOriginEntity[i].sY;
					a3=pData->m_pLayer[nLayerNum].pOriginEntity[i].eX;
					a4=pData->m_pLayer[nLayerNum].pOriginEntity[i].eY;
					a1=a1*m_zoom*m_dxfzoom+m_right;
					a2=View_Y-a2*m_zoom*m_dxfzoom+m_updown;
					a3=a3*m_zoom*m_dxfzoom+m_right;
					a4=View_Y-a4*m_zoom*m_dxfzoom+m_updown;


					bool bResultTotal = true;

 					if( ( Change_int(a1) < 0	||		Change_int(a1) > View_X	) &&
 						( Change_int(a3) < 0	||		Change_int(a3) > View_X	) &&
 						( Change_int(a2) < 0	||		Change_int(a2) > View_Y	) &&
 						( Change_int(a4) < 0	||		Change_int(a4) > View_Y	)	)		 					
						bResultTotal = false;

					if( pData->Calculate2PointDistance( a1, a2, a3, a4 ) < 1 )
						break;


					if(bResultTotal == false)
						break;

					MemDC.DrawLine(&penArc, int(a1), int(a2), int(a3), int(a4));
				}
				break;
			case 3: //rectangle
				break;
			case _ENTITY_CIRCLE: //circle
				a1=pData->m_pLayer[nLayerNum].pOriginEntity[i].cX;
				a2=pData->m_pLayer[nLayerNum].pOriginEntity[i].cY;
				a3=pData->m_pLayer[nLayerNum].pOriginEntity[i].rad;

				a1=a1*m_zoom*m_dxfzoom+m_right;
				a2=View_Y-a2*m_zoom*m_dxfzoom+m_updown;
				a3=a3*m_zoom*m_dxfzoom;

				if( ( Change_int(a1+a3) < 0			||		Change_int(a1+a3) > View_X	) &&	
					( Change_int(a1-a3) < 0			||		Change_int(a1-a3) > View_X	) &&	
					( Change_int(a2+a3) < 0			||		Change_int(a2+a3) > View_Y	) &&	
					( Change_int(a2-a3) < 0			||		Change_int(a2-a3) > View_Y	) )
					break;

				MemDC.DrawArc(&penArc, float(a1-a3), float(a2-a3), float(a3*2), float(a3*2), 0, 360);
				break;
			case _ENTITY_ARC: //arc
				{
										a1=pData->m_pLayer[nLayerNum].pOriginEntity[i].cX;
					a2=pData->m_pLayer[nLayerNum].pOriginEntity[i].cY;
					a3=pData->m_pLayer[nLayerNum].pOriginEntity[i].rad;
					a4=pData->m_pLayer[nLayerNum].pOriginEntity[i].sA;
					a5=pData->m_pLayer[nLayerNum].pOriginEntity[i].eA;

					double sX, sY;
					double eX, eY;
					//  				sX = a1+(cos(a4*PHI/180.0))*a3;
					//  				sY = a2+(sin(a4*PHI/180.0))*a3;
					//  				eX = a1+(cos(a5*PHI/180.0))*a3;
					//  				eY = a2+(sin(a5*PHI/180.0))*a3;

					sX = pData->m_pLayer[nLayerNum].pOriginEntity[i].sX;
					eX = pData->m_pLayer[nLayerNum].pOriginEntity[i].eX;
					sY = pData->m_pLayer[nLayerNum].pOriginEntity[i].sY;
					eY = pData->m_pLayer[nLayerNum].pOriginEntity[i].eY;

					sX=sX*m_zoom*m_dxfzoom+m_right;
					sY=View_Y-sY*m_zoom*m_dxfzoom+m_updown;
					eX=eX*m_zoom*m_dxfzoom+m_right;
					eY=View_Y-eY*m_zoom*m_dxfzoom+m_updown;

					a1=a1*m_zoom*m_dxfzoom+m_right;
					a2=View_Y-a2*m_zoom*m_dxfzoom+m_updown ;
					a3=a3*m_zoom*m_dxfzoom;

					//VIEWER에서는 Y축이 반전되어 각도를 변환시켜줘야함.<360 - ANGLE>
					a4 = 360 - pData->m_pLayer[nLayerNum].pOriginEntity[i].sA;
					a5 = 360 - pData->m_pLayer[nLayerNum].pOriginEntity[i].eA;
					
					REAL a[] = {(float)(a1-a3), (float)(a2-a3), (float)(2*a3), (float)(2*a3), (float)a4, (float)(a5-a4)};
					
					MemDC.DrawArc(&penArc,a[0], a[1], a[2], a[3], a[4], a[5] );
	//				}
				}
				break;

			case _ENTITY_ELLIPSE: //arc
				{
					a1=pData->m_pLayer[nLayerNum].pOriginEntity[i].sX;
					a2=pData->m_pLayer[nLayerNum].pOriginEntity[i].sY;
					a3=pData->m_pLayer[nLayerNum].pOriginEntity[i].rad;
					a4=pData->m_pLayer[nLayerNum].pOriginEntity[i].sA;
					a5=pData->m_pLayer[nLayerNum].pOriginEntity[i].eA;
					a1=a1*m_zoom*m_dxfzoom+m_right;
					a2=View_Y-a2*m_zoom*m_dxfzoom+m_updown;
					a3=a3*m_zoom*m_dxfzoom;
					a4=360-a4;
					a5=360-a5;

					double sx, sy, ex, ey, radAng, temp_x,temp_y,temp_sa,temp_ea, ratio, cx, cy, radRotation;
					bool bStop = false;
					double add_i;
					double A, B;

					temp_sa = pData->m_pLayer[nLayerNum].pOriginEntity[i].sA;
					temp_ea = pData->m_pLayer[nLayerNum].pOriginEntity[i].eA;

					ratio = pData->m_pLayer[nLayerNum].pOriginEntity[i].rad;

					sx = pData->m_pLayer[nLayerNum].pOriginEntity[i].sX;
					sy = pData->m_pLayer[nLayerNum].pOriginEntity[i].sY;

					cx = pData->m_pLayer[nLayerNum].pOriginEntity[i].cX;
					cy = pData->m_pLayer[nLayerNum].pOriginEntity[i].cY;

					A = pData->Calculate2PointDistance(sx, sy, cx, cy); //장축
					B = A*ratio;								//단축

					radRotation = atan2(sy - cy, sx - cx);// * 180. / PHI;
					double zz = radRotation *180 / PHI;

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

					for(;;)
					{
						temp_sa += add_i;

						if(pData->DoRound(temp_sa)>=pData->DoRound(temp_ea)) 
						{
							temp_sa = temp_ea;
							bStop = true;
						}

						radAng = (temp_sa) * PHI / 180.;

						temp_x = cx + A * cos(radAng)*cos(radRotation) - B * sin(radAng) * sin(radRotation);
						temp_y = cy + A * cos(radAng)*sin(radRotation) + B * sin(radAng) * cos(radRotation);

						ex = temp_x;
						ey = temp_y;

						MemDC.DrawLine( &penArc, float(temp_x * m_zoom*m_dxfzoom+m_right), float( View_Y - temp_y * m_zoom*m_dxfzoom+m_updown)
							, float(ex * m_zoom*m_dxfzoom+m_right), float(View_Y - ey * m_zoom*m_dxfzoom+m_updown) );

						sx = ex;
						sy = ey;

						if(bStop) break;
					}
				}

				break;

			case 7: //arc
				break;
			}
		}
	}

	
	if(bool_sector==TRUE)
		View_Draw_Sector(&MemDC);

	if(m_bOrderView == TRUE){
		View_Draw_SectorOrder(&MemDC);
	}
	
	if(bool_change_sector==TRUE)
	{
		View_Draw_SectorLine(&MemDC);
		//if(ClickFlag == TRUE)
		//	return;
	}

	*/
	/*
	dc. BitBlt(0,0, rt.Width(), rt.Height(), &MemDC, 0, 0, SRCCOPY);

	delete[] penT;*/
	//for(int i=-100; i<100; i+=5)
	//	gTools.DrawImage(&bitmap, 0, 0);
}

void CFileEntityViewer::Initialize()
{
	m_zoom			= 1;
	m_dxfzoom		= 1;
	m_right			= 0;
	m_updown		= 0;
	Mini_Rectx		= 0;
	Mini_Recty		= 0;

	buffer_count	= 0;

	m_viewer_abs_x	= "0";
	m_viewer_abs_y	= "0";

	m_MinX			= 999999;
	m_MinY			= 999999;
	m_MaxX			= -999999;
	m_MaxY			= -999999;

	bool_orgview	=FALSE;
	bool_absview	=FALSE;
	bool_absmove	=FALSE;
	bool_absmove2	=FALSE;
	bool_sector		=FALSE;
	bool_change_sector=FALSE;
	m_bOrderView = FALSE;
	m_bOrderModify = FALSE;
	ClickFlag		 = FALSE;

	double low_pointx = pData->m_fTotalMinX;
	double low_pointy = pData->m_fTotalMinY;
	double high_pointx = pData->m_fTotalMaxX;
	double high_pointy = pData->m_fTotalMaxY;
	double center_pointx = (low_pointx + high_pointx)/2;
	double center_pointy = ((low_pointy + high_pointy)/2);

	///뷰활성화 부분
	double plt_width = high_pointx-low_pointx;
	double plt_height = high_pointy-low_pointy;

	//화면에 가득차게 비율을 조정한다.
	if(plt_width/plt_height*View_Y/View_X>1)
		m_dxfzoom = (View_X-60)/plt_width;
	else
		m_dxfzoom = (View_Y-60)/plt_height;

	double cx=(int)((high_pointx*m_dxfzoom+low_pointx*m_dxfzoom)/2);
	double cy=(int)((high_pointy*m_dxfzoom+low_pointy*m_dxfzoom)/2);

	m_updown=(((View_Y/2)-((int)(cy))));
	m_right =(((View_X/2)-((int)(cx))));

	if(m_bViewLayer != NULL)
	{
		delete[] m_bViewLayer;
		m_bViewLayer = NULL;
	}
	m_bViewLayer = new BOOL[pData->m_iLayerCount];
	memset(m_bViewLayer, TRUE, sizeof(BOOL) * pData->m_iLayerCount);

	InitInputNumMEM();

	Invalidate();
}

void CFileEntityViewer::SetSectorSize(int number)
{
	m_SectorSize = number;
}


int CFileEntityViewer::Layer_Color(int color)
{
	switch (color){
	case 0:
		m_Layer_Color.C_R=104;m_Layer_Color.C_G=0;m_Layer_Color.C_B=0;
		break;
	case 1:
		m_Layer_Color.C_R=255;m_Layer_Color.C_G=0;m_Layer_Color.C_B=0;
		break;
	case 2:
		m_Layer_Color.C_R=255;m_Layer_Color.C_G=255;m_Layer_Color.C_B=0;
		break;
	case 3:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=255;m_Layer_Color.C_B=0;
		break;
	case 4:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=255;m_Layer_Color.C_B=255;
		break;
	case 5:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=0;m_Layer_Color.C_B=255;
		break;
	case 6:
		m_Layer_Color.C_R=255;m_Layer_Color.C_G=0;m_Layer_Color.C_B=255;
		break;
	case 7:
		m_Layer_Color.C_R=255;m_Layer_Color.C_G=255;m_Layer_Color.C_B=255;
		break;
	case 8:
		m_Layer_Color.C_R=65;m_Layer_Color.C_G=65;m_Layer_Color.C_B=65;
		break;
	case 9:
		m_Layer_Color.C_R=128;m_Layer_Color.C_G=128;m_Layer_Color.C_B=128;
		break;
	case 10:
		m_Layer_Color.C_R=255;m_Layer_Color.C_G=0;m_Layer_Color.C_B=0;
		break;
	case 11:
		m_Layer_Color.C_R=255;m_Layer_Color.C_G=170;m_Layer_Color.C_B=170;
		break;
	case 12:
		m_Layer_Color.C_R=189;m_Layer_Color.C_G=0;m_Layer_Color.C_B=0;
		break;
	case 13:
		m_Layer_Color.C_R=189;m_Layer_Color.C_G=126;m_Layer_Color.C_B=126;
		break;
	case 14:
		m_Layer_Color.C_R=129;m_Layer_Color.C_G=0;m_Layer_Color.C_B=0;
		break;
	case 15:
		m_Layer_Color.C_R=129;m_Layer_Color.C_G=86;m_Layer_Color.C_B=86;
		break;
	case 16:
		m_Layer_Color.C_R=104;m_Layer_Color.C_G=0;m_Layer_Color.C_B=0;
		break;
	case 17:
		m_Layer_Color.C_R=104;m_Layer_Color.C_G=69;m_Layer_Color.C_B=69;
		break;
	case 18:
		m_Layer_Color.C_R=79;m_Layer_Color.C_G=0;m_Layer_Color.C_B=0;
		break;
	case 19:
		m_Layer_Color.C_R=79;m_Layer_Color.C_G=53;m_Layer_Color.C_B=53;
		break;
	case 20:
		m_Layer_Color.C_R=255;m_Layer_Color.C_G=63;m_Layer_Color.C_B=0;
		break;
	case 21:
		m_Layer_Color.C_R=255;m_Layer_Color.C_G=191;m_Layer_Color.C_B=170;
		break;
	case 22:
		m_Layer_Color.C_R=189;m_Layer_Color.C_G=46;m_Layer_Color.C_B=0;
		break;
	case 23:
		m_Layer_Color.C_R=189;m_Layer_Color.C_G=141;m_Layer_Color.C_B=126;
		break;
	case 24:
		m_Layer_Color.C_R=129;m_Layer_Color.C_G=31;m_Layer_Color.C_B=0;
		break;
	case 25:
		m_Layer_Color.C_R=129;m_Layer_Color.C_G=96;m_Layer_Color.C_B=86;
		break;
	case 26:
		m_Layer_Color.C_R=104;m_Layer_Color.C_G=25;m_Layer_Color.C_B=0;
		break;
	case 27:
		m_Layer_Color.C_R=104;m_Layer_Color.C_G=78;m_Layer_Color.C_B=69;
		break;
	case 28:
		m_Layer_Color.C_R=79;m_Layer_Color.C_G=19;m_Layer_Color.C_B=0;
		break;
	case 29:
		m_Layer_Color.C_R=79;m_Layer_Color.C_G=59;m_Layer_Color.C_B=53;
		break;
	case 30:
		m_Layer_Color.C_R=255;m_Layer_Color.C_G=127;m_Layer_Color.C_B=0;
		break;
	case 31:
		m_Layer_Color.C_R=255;m_Layer_Color.C_G=212;m_Layer_Color.C_B=170;
		break;
	case 32:
		m_Layer_Color.C_R=189;m_Layer_Color.C_G=94;m_Layer_Color.C_B=0;
		break;
	case 33:
		m_Layer_Color.C_R=189;m_Layer_Color.C_G=157;m_Layer_Color.C_B=126;
		break;
	case 34:
		m_Layer_Color.C_R=129;m_Layer_Color.C_G=64;m_Layer_Color.C_B=0;
		break;
	case 35:
		m_Layer_Color.C_R=129;m_Layer_Color.C_G=107;m_Layer_Color.C_B=86;
		break;
	case 36:
		m_Layer_Color.C_R=104;m_Layer_Color.C_G=52;m_Layer_Color.C_B=0;
		break;
	case 37:
		m_Layer_Color.C_R=104;m_Layer_Color.C_G=86;m_Layer_Color.C_B=69;
		break;
	case 38:
		m_Layer_Color.C_R=79;m_Layer_Color.C_G=39;m_Layer_Color.C_B=0;
		break;
	case 39:
		m_Layer_Color.C_R=79;m_Layer_Color.C_G=66;m_Layer_Color.C_B=53;
		break;
	case 40:
		m_Layer_Color.C_R=255;m_Layer_Color.C_G=191;m_Layer_Color.C_B=0;
		break;
	case 41:
		m_Layer_Color.C_R=255;m_Layer_Color.C_G=234;m_Layer_Color.C_B=170;
		break;
	case 42:
		m_Layer_Color.C_R=189;m_Layer_Color.C_G=141;m_Layer_Color.C_B=0;
		break;
	case 43:
		m_Layer_Color.C_R=189;m_Layer_Color.C_G=173;m_Layer_Color.C_B=126;
		break;
	case 44:
		m_Layer_Color.C_R=129;m_Layer_Color.C_G=96;m_Layer_Color.C_B=0;
		break;
	case 45:
		m_Layer_Color.C_R=129;m_Layer_Color.C_G=118;m_Layer_Color.C_B=86;
		break;
	case 46:
		m_Layer_Color.C_R=104;m_Layer_Color.C_G=78;m_Layer_Color.C_B=0;
		break;
	case 47:
		m_Layer_Color.C_R=104;m_Layer_Color.C_G=95;m_Layer_Color.C_B=69;
		break;
	case 48:
		m_Layer_Color.C_R=79;m_Layer_Color.C_G=59;m_Layer_Color.C_B=0;
		break;
	case 49:
		m_Layer_Color.C_R=79;m_Layer_Color.C_G=73;m_Layer_Color.C_B=53;
		break;
	case 50:
		m_Layer_Color.C_R=255;m_Layer_Color.C_G=255;m_Layer_Color.C_B=0;
		break;
	case 51:
		m_Layer_Color.C_R=255;m_Layer_Color.C_G=255;m_Layer_Color.C_B=170;
		break;
	case 52:
		m_Layer_Color.C_R=189;m_Layer_Color.C_G=189;m_Layer_Color.C_B=0;
		break;
	case 53:
		m_Layer_Color.C_R=189;m_Layer_Color.C_G=189;m_Layer_Color.C_B=126;
		break;
	case 54:
		m_Layer_Color.C_R=129;m_Layer_Color.C_G=129;m_Layer_Color.C_B=0;
		break;
	case 55:
		m_Layer_Color.C_R=129;m_Layer_Color.C_G=129;m_Layer_Color.C_B=86;
		break;
	case 56:
		m_Layer_Color.C_R=104;m_Layer_Color.C_G=104;m_Layer_Color.C_B=0;
		break;
	case 57:
		m_Layer_Color.C_R=104;m_Layer_Color.C_G=104;m_Layer_Color.C_B=69;
		break;
	case 58:
		m_Layer_Color.C_R=79;m_Layer_Color.C_G=79;m_Layer_Color.C_B=0;
		break;
	case 59:
		m_Layer_Color.C_R=79;m_Layer_Color.C_G=79;m_Layer_Color.C_B=53;
		break;
	case 60:
		m_Layer_Color.C_R=191;m_Layer_Color.C_G=255;m_Layer_Color.C_B=0;
		break;
	case 61:
		m_Layer_Color.C_R=234;m_Layer_Color.C_G=255;m_Layer_Color.C_B=170;
		break;
	case 62:
		m_Layer_Color.C_R=141;m_Layer_Color.C_G=189;m_Layer_Color.C_B=0;
		break;
	case 63:
		m_Layer_Color.C_R=173;m_Layer_Color.C_G=189;m_Layer_Color.C_B=126;
		break;
	case 64:
		m_Layer_Color.C_R=96;m_Layer_Color.C_G=129;m_Layer_Color.C_B=0;
		break;
	case 65:
		m_Layer_Color.C_R=118;m_Layer_Color.C_G=129;m_Layer_Color.C_B=86;
		break;
	case 66:
		m_Layer_Color.C_R=78;m_Layer_Color.C_G=104;m_Layer_Color.C_B=0;
		break;
	case 67:
		m_Layer_Color.C_R=95;m_Layer_Color.C_G=104;m_Layer_Color.C_B=69;
		break;
	case 68:
		m_Layer_Color.C_R=59;m_Layer_Color.C_G=79;m_Layer_Color.C_B=0;
		break;
	case 69:
		m_Layer_Color.C_R=73;m_Layer_Color.C_G=79;m_Layer_Color.C_B=53;
		break;
	case 70:
		m_Layer_Color.C_R=127;m_Layer_Color.C_G=255;m_Layer_Color.C_B=0;
		break;
	case 71:
		m_Layer_Color.C_R=212;m_Layer_Color.C_G=255;m_Layer_Color.C_B=170;
		break;
	case 72:
		m_Layer_Color.C_R=94;m_Layer_Color.C_G=189;m_Layer_Color.C_B=0;
		break;
	case 73:
		m_Layer_Color.C_R=157;m_Layer_Color.C_G=189;m_Layer_Color.C_B=126;
		break;
	case 74:
		m_Layer_Color.C_R=64;m_Layer_Color.C_G=129;m_Layer_Color.C_B=0;
		break;
	case 75:
		m_Layer_Color.C_R=107;m_Layer_Color.C_G=129;m_Layer_Color.C_B=86;
		break;
	case 76:
		m_Layer_Color.C_R=52;m_Layer_Color.C_G=104;m_Layer_Color.C_B=0;
		break;
	case 77:
		m_Layer_Color.C_R=86;m_Layer_Color.C_G=104;m_Layer_Color.C_B=69;
		break;
	case 78:
		m_Layer_Color.C_R=39;m_Layer_Color.C_G=79;m_Layer_Color.C_B=0;
		break;
	case 79:
		m_Layer_Color.C_R=66;m_Layer_Color.C_G=79;m_Layer_Color.C_B=53;
		break;
	case 80:
		m_Layer_Color.C_R=63;m_Layer_Color.C_G=255;m_Layer_Color.C_B=0;
		break;
	case 81:
		m_Layer_Color.C_R=191;m_Layer_Color.C_G=255;m_Layer_Color.C_B=170;
		break;
	case 82:
		m_Layer_Color.C_R=46;m_Layer_Color.C_G=189;m_Layer_Color.C_B=0;
		break;
	case 83:
		m_Layer_Color.C_R=141;m_Layer_Color.C_G=189;m_Layer_Color.C_B=126;
		break;
	case 84:
		m_Layer_Color.C_R=31;m_Layer_Color.C_G=129;m_Layer_Color.C_B=0;
		break;
	case 85:
		m_Layer_Color.C_R=96;m_Layer_Color.C_G=129;m_Layer_Color.C_B=86;
		break;
	case 86:
		m_Layer_Color.C_R=25;m_Layer_Color.C_G=104;m_Layer_Color.C_B=0;
		break;
	case 87:
		m_Layer_Color.C_R=78;m_Layer_Color.C_G=104;m_Layer_Color.C_B=69;
		break;
	case 88:
		m_Layer_Color.C_R=19;m_Layer_Color.C_G=79;m_Layer_Color.C_B=0;
		break;
	case 89:
		m_Layer_Color.C_R=59;m_Layer_Color.C_G=79;m_Layer_Color.C_B=53;
		break;
	case 90:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=255;m_Layer_Color.C_B=0;
		break;
	case 91:
		m_Layer_Color.C_R=170;m_Layer_Color.C_G=255;m_Layer_Color.C_B=170;
		break;
	case 92:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=189;m_Layer_Color.C_B=0;
		break;
	case 93:
		m_Layer_Color.C_R=126;m_Layer_Color.C_G=189;m_Layer_Color.C_B=126;
		break;
	case 94:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=129;m_Layer_Color.C_B=0;
		break;
	case 95:
		m_Layer_Color.C_R=86;m_Layer_Color.C_G=129;m_Layer_Color.C_B=86;
		break;
	case 96:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=104;m_Layer_Color.C_B=0;
		break;
	case 97:
		m_Layer_Color.C_R=69;m_Layer_Color.C_G=104;m_Layer_Color.C_B=69;
		break;
	case 98:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=79;m_Layer_Color.C_B=0;
		break;
	case 99:
		m_Layer_Color.C_R=53;m_Layer_Color.C_G=79;m_Layer_Color.C_B=53;
		break;
	case 100:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=255;m_Layer_Color.C_B=63;
		break;
	case 101:
		m_Layer_Color.C_R=170;m_Layer_Color.C_G=255;m_Layer_Color.C_B=191;
		break;
	case 102:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=189;m_Layer_Color.C_B=46;
		break;
	case 103:
		m_Layer_Color.C_R=126;m_Layer_Color.C_G=189;m_Layer_Color.C_B=141;
		break;
	case 104:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=129;m_Layer_Color.C_B=31;
		break;
	case 105:
		m_Layer_Color.C_R=86;m_Layer_Color.C_G=129;m_Layer_Color.C_B=96;
		break;
	case 106:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=104;m_Layer_Color.C_B=25;
		break;
	case 107:
		m_Layer_Color.C_R=69;m_Layer_Color.C_G=104;m_Layer_Color.C_B=78;
		break;
	case 108:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=79;m_Layer_Color.C_B=19;
		break;
	case 109:
		m_Layer_Color.C_R=53;m_Layer_Color.C_G=79;m_Layer_Color.C_B=59;
		break;
	case 110:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=255;m_Layer_Color.C_B=127;
		break;
	case 111:
		m_Layer_Color.C_R=170;m_Layer_Color.C_G=255;m_Layer_Color.C_B=212;
		break;
	case 112:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=189;m_Layer_Color.C_B=94;
		break;
	case 113:
		m_Layer_Color.C_R=126;m_Layer_Color.C_G=189;m_Layer_Color.C_B=157;
		break;
	case 114:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=129;m_Layer_Color.C_B=64;
		break;
	case 115:
		m_Layer_Color.C_R=86;m_Layer_Color.C_G=129;m_Layer_Color.C_B=107;
		break;
	case 116:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=104;m_Layer_Color.C_B=52;
		break;
	case 117:
		m_Layer_Color.C_R=69;m_Layer_Color.C_G=104;m_Layer_Color.C_B=86;
		break;
	case 118:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=79;m_Layer_Color.C_B=39;
		break;
	case 119:
		m_Layer_Color.C_R=53;m_Layer_Color.C_G=79;m_Layer_Color.C_B=66;
		break;
	case 120:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=255;m_Layer_Color.C_B=191;
		break;
	case 121:
		m_Layer_Color.C_R=170;m_Layer_Color.C_G=255;m_Layer_Color.C_B=234;
		break;
	case 122:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=189;m_Layer_Color.C_B=141;
		break;
	case 123:
		m_Layer_Color.C_R=126;m_Layer_Color.C_G=189;m_Layer_Color.C_B=173;
		break;
	case 124:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=129;m_Layer_Color.C_B=96;
		break;
	case 125:
		m_Layer_Color.C_R=86;m_Layer_Color.C_G=129;m_Layer_Color.C_B=118;
		break;
	case 126:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=104;m_Layer_Color.C_B=78;
		break;
	case 127:
		m_Layer_Color.C_R=69;m_Layer_Color.C_G=104;m_Layer_Color.C_B=95;
		break;
	case 128:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=79;m_Layer_Color.C_B=59;
		break;
	case 129:
		m_Layer_Color.C_R=53;m_Layer_Color.C_G=79;m_Layer_Color.C_B=73;
		break;
	case 130:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=255;m_Layer_Color.C_B=255;
		break;
	case 131:
		m_Layer_Color.C_R=170;m_Layer_Color.C_G=255;m_Layer_Color.C_B=255;
		break;
	case 132:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=189;m_Layer_Color.C_B=189;
		break;
	case 133:
		m_Layer_Color.C_R=126;m_Layer_Color.C_G=189;m_Layer_Color.C_B=189;
		break;
	case 134:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=129;m_Layer_Color.C_B=129;
		break;
	case 135:
		m_Layer_Color.C_R=86;m_Layer_Color.C_G=129;m_Layer_Color.C_B=129;
		break;
	case 136:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=104;m_Layer_Color.C_B=104;
		break;
	case 137:
		m_Layer_Color.C_R=69;m_Layer_Color.C_G=104;m_Layer_Color.C_B=104;
		break;
	case 138:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=79;m_Layer_Color.C_B=79;
		break;
	case 139:
		m_Layer_Color.C_R=53;m_Layer_Color.C_G=79;m_Layer_Color.C_B=79;
		break;
	case 140:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=191;m_Layer_Color.C_B=255;
		break;
	case 141:
		m_Layer_Color.C_R=170;m_Layer_Color.C_G=234;m_Layer_Color.C_B=255;
		break;
	case 142:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=141;m_Layer_Color.C_B=189;
		break;
	case 143:
		m_Layer_Color.C_R=126;m_Layer_Color.C_G=173;m_Layer_Color.C_B=189;
		break;
	case 144:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=96;m_Layer_Color.C_B=129;
		break;
	case 145:
		m_Layer_Color.C_R=86;m_Layer_Color.C_G=118;m_Layer_Color.C_B=129;
		break;
	case 146:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=78;m_Layer_Color.C_B=104;
		break;
	case 147:
		m_Layer_Color.C_R=69;m_Layer_Color.C_G=95;m_Layer_Color.C_B=104;
		break;
	case 148:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=59;m_Layer_Color.C_B=79;
		break;
	case 149:
		m_Layer_Color.C_R=53;m_Layer_Color.C_G=73;m_Layer_Color.C_B=79;
		break;
	case 150:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=127;m_Layer_Color.C_B=255;
		break;
	case 151:
		m_Layer_Color.C_R=170;m_Layer_Color.C_G=212;m_Layer_Color.C_B=255;
		break;
	case 152:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=94;m_Layer_Color.C_B=189;
		break;
	case 153:
		m_Layer_Color.C_R=126;m_Layer_Color.C_G=157;m_Layer_Color.C_B=189;
		break;
	case 154:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=64;m_Layer_Color.C_B=129;
		break;
	case 155:
		m_Layer_Color.C_R=86;m_Layer_Color.C_G=107;m_Layer_Color.C_B=129;
		break;
	case 156:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=52;m_Layer_Color.C_B=104;
		break;
	case 157:
		m_Layer_Color.C_R=69;m_Layer_Color.C_G=86;m_Layer_Color.C_B=104;
		break;
	case 158:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=39;m_Layer_Color.C_B=79;
		break;
	case 159:
		m_Layer_Color.C_R=53;m_Layer_Color.C_G=66;m_Layer_Color.C_B=79;
		break;
	case 160:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=63;m_Layer_Color.C_B=255;
		break;
	case 161:
		m_Layer_Color.C_R=170;m_Layer_Color.C_G=191;m_Layer_Color.C_B=255;
		break;
	case 162:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=46;m_Layer_Color.C_B=189;
		break;
	case 163:
		m_Layer_Color.C_R=126;m_Layer_Color.C_G=141;m_Layer_Color.C_B=189;
		break;
	case 164:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=31;m_Layer_Color.C_B=129;
		break;
	case 165:
		m_Layer_Color.C_R=86;m_Layer_Color.C_G=96;m_Layer_Color.C_B=129;
		break;
	case 166:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=25;m_Layer_Color.C_B=104;
		break;
	case 167:
		m_Layer_Color.C_R=69;m_Layer_Color.C_G=78;m_Layer_Color.C_B=104;
		break;
	case 168:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=19;m_Layer_Color.C_B=79;
		break;
	case 169:
		m_Layer_Color.C_R=53;m_Layer_Color.C_G=59;m_Layer_Color.C_B=79;
		break;
	case 170:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=0;m_Layer_Color.C_B=255;
		break;
	case 171:
		m_Layer_Color.C_R=170;m_Layer_Color.C_G=170;m_Layer_Color.C_B=255;
		break;
	case 172:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=0;m_Layer_Color.C_B=189;
		break;
	case 173:
		m_Layer_Color.C_R=126;m_Layer_Color.C_G=126;m_Layer_Color.C_B=189;
		break;
	case 174:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=0;m_Layer_Color.C_B=129;
		break;
	case 175:
		m_Layer_Color.C_R=86;m_Layer_Color.C_G=86;m_Layer_Color.C_B=129;
		break;
	case 176:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=0;m_Layer_Color.C_B=104;
		break;
	case 177:
		m_Layer_Color.C_R=69;m_Layer_Color.C_G=69;m_Layer_Color.C_B=104;
		break;
	case 178:
		m_Layer_Color.C_R=0;m_Layer_Color.C_G=0;m_Layer_Color.C_B=79;
		break;
	case 179:
		m_Layer_Color.C_R=53;m_Layer_Color.C_G=53;m_Layer_Color.C_B=79;
		break;
	case 180:
		m_Layer_Color.C_R=63;m_Layer_Color.C_G=0;m_Layer_Color.C_B=255;
		break;
	case 181:
		m_Layer_Color.C_R=191;m_Layer_Color.C_G=170;m_Layer_Color.C_B=255;
		break;
	case 182:
		m_Layer_Color.C_R=46;m_Layer_Color.C_G=0;m_Layer_Color.C_B=189;
		break;
	case 183:
		m_Layer_Color.C_R=141;m_Layer_Color.C_G=126;m_Layer_Color.C_B=189;
		break;
	case 184:
		m_Layer_Color.C_R=31;m_Layer_Color.C_G=0;m_Layer_Color.C_B=129;
		break;
	case 185:
		m_Layer_Color.C_R=96;m_Layer_Color.C_G=86;m_Layer_Color.C_B=129;
		break;
	case 186:
		m_Layer_Color.C_R=25;m_Layer_Color.C_G=0;m_Layer_Color.C_B=104;
		break;
	case 187:
		m_Layer_Color.C_R=78;m_Layer_Color.C_G=69;m_Layer_Color.C_B=104;
		break;
	case 188:
		m_Layer_Color.C_R=19;m_Layer_Color.C_G=0;m_Layer_Color.C_B=79;
		break;
	case 189:
		m_Layer_Color.C_R=59;m_Layer_Color.C_G=53;m_Layer_Color.C_B=79;
		break;
	case 190:
		m_Layer_Color.C_R=127;m_Layer_Color.C_G=0;m_Layer_Color.C_B=255;
		break;
	case 191:
		m_Layer_Color.C_R=212;m_Layer_Color.C_G=170;m_Layer_Color.C_B=255;
		break;
	case 192:
		m_Layer_Color.C_R=94;m_Layer_Color.C_G=0;m_Layer_Color.C_B=189;
		break;
	case 193:
		m_Layer_Color.C_R=157;m_Layer_Color.C_G=126;m_Layer_Color.C_B=189;
		break;
	case 194:
		m_Layer_Color.C_R=64;m_Layer_Color.C_G=0;m_Layer_Color.C_B=129;
		break;
	case 195:
		m_Layer_Color.C_R=107;m_Layer_Color.C_G=86;m_Layer_Color.C_B=129;
		break;
	case 196:
		m_Layer_Color.C_R=52;m_Layer_Color.C_G=0;m_Layer_Color.C_B=104;
		break;
	case 197:
		m_Layer_Color.C_R=86;m_Layer_Color.C_G=69;m_Layer_Color.C_B=104;
		break;
	case 198:
		m_Layer_Color.C_R=39;m_Layer_Color.C_G=0;m_Layer_Color.C_B=79;
		break;
	case 199:
		m_Layer_Color.C_R=66;m_Layer_Color.C_G=53;m_Layer_Color.C_B=79;
		break;
	case 200:
		m_Layer_Color.C_R=191;m_Layer_Color.C_G=0;m_Layer_Color.C_B=255;
		break;
	case 201:
		m_Layer_Color.C_R=234;m_Layer_Color.C_G=170;m_Layer_Color.C_B=255;
		break;
	case 202:
		m_Layer_Color.C_R=141;m_Layer_Color.C_G=0;m_Layer_Color.C_B=189;
		break;
	case 203:
		m_Layer_Color.C_R=173;m_Layer_Color.C_G=126;m_Layer_Color.C_B=189;
		break;
	case 204:
		m_Layer_Color.C_R=96;m_Layer_Color.C_G=0;m_Layer_Color.C_B=129;
		break;
	case 205:
		m_Layer_Color.C_R=118;m_Layer_Color.C_G=86;m_Layer_Color.C_B=129;
		break;
	case 206:
		m_Layer_Color.C_R=78;m_Layer_Color.C_G=0;m_Layer_Color.C_B=104;
		break;
	case 207:
		m_Layer_Color.C_R=95;m_Layer_Color.C_G=69;m_Layer_Color.C_B=104;
		break;
	case 208:
		m_Layer_Color.C_R=59;m_Layer_Color.C_G=0;m_Layer_Color.C_B=79;
		break;
	case 209:
		m_Layer_Color.C_R=73;m_Layer_Color.C_G=53;m_Layer_Color.C_B=79;
		break;
	case 210:
		m_Layer_Color.C_R=255;m_Layer_Color.C_G=0;m_Layer_Color.C_B=255;
		break;
	case 211:
		m_Layer_Color.C_R=255;m_Layer_Color.C_G=170;m_Layer_Color.C_B=255;
		break;
	case 212:
		m_Layer_Color.C_R=189;m_Layer_Color.C_G=0;m_Layer_Color.C_B=189;
		break;
	case 213:
		m_Layer_Color.C_R=189;m_Layer_Color.C_G=126;m_Layer_Color.C_B=189;
		break;
	case 214:
		m_Layer_Color.C_R=129;m_Layer_Color.C_G=0;m_Layer_Color.C_B=129;
		break;
	case 215:
		m_Layer_Color.C_R=129;m_Layer_Color.C_G=86;m_Layer_Color.C_B=129;
		break;
	case 216:
		m_Layer_Color.C_R=104;m_Layer_Color.C_G=0;m_Layer_Color.C_B=104;
		break;
	case 217:
		m_Layer_Color.C_R=104;m_Layer_Color.C_G=69;m_Layer_Color.C_B=104;
		break;
	case 218:
		m_Layer_Color.C_R=79;m_Layer_Color.C_G=0;m_Layer_Color.C_B=79;
		break;
	case 219:
		m_Layer_Color.C_R=79;m_Layer_Color.C_G=53;m_Layer_Color.C_B=79;
		break;
	case 220:
		m_Layer_Color.C_R=255;m_Layer_Color.C_G=0;m_Layer_Color.C_B=191;
		break;
	case 221:
		m_Layer_Color.C_R=255;m_Layer_Color.C_G=170;m_Layer_Color.C_B=234;
		break;
	case 222:
		m_Layer_Color.C_R=189;m_Layer_Color.C_G=0;m_Layer_Color.C_B=141;
		break;
	case 223:
		m_Layer_Color.C_R=189;m_Layer_Color.C_G=126;m_Layer_Color.C_B=173;
		break;
	case 224:
		m_Layer_Color.C_R=129;m_Layer_Color.C_G=0;m_Layer_Color.C_B=96;
		break;
	case 225:
		m_Layer_Color.C_R=129;m_Layer_Color.C_G=86;m_Layer_Color.C_B=118;
		break;
	case 226:
		m_Layer_Color.C_R=104;m_Layer_Color.C_G=0;m_Layer_Color.C_B=78;
		break;
	case 227:
		m_Layer_Color.C_R=104;m_Layer_Color.C_G=69;m_Layer_Color.C_B=95;
		break;
	case 228:
		m_Layer_Color.C_R=79;m_Layer_Color.C_G=0;m_Layer_Color.C_B=59;
		break;
	case 229:
		m_Layer_Color.C_R=79;m_Layer_Color.C_G=53;m_Layer_Color.C_B=73;
		break;
	case 230:
		m_Layer_Color.C_R=255;m_Layer_Color.C_G=0;m_Layer_Color.C_B=127;
		break;
	case 231:
		m_Layer_Color.C_R=255;m_Layer_Color.C_G=170;m_Layer_Color.C_B=212;
		break;
	case 232:
		m_Layer_Color.C_R=189;m_Layer_Color.C_G=0;m_Layer_Color.C_B=94;
		break;
	case 233:
		m_Layer_Color.C_R=189;m_Layer_Color.C_G=126;m_Layer_Color.C_B=157;
		break;
	case 234:
		m_Layer_Color.C_R=129;m_Layer_Color.C_G=0;m_Layer_Color.C_B=64;
		break;
	case 235:
		m_Layer_Color.C_R=129;m_Layer_Color.C_G=86;m_Layer_Color.C_B=107;
		break;
	case 236:
		m_Layer_Color.C_R=104;m_Layer_Color.C_G=0;m_Layer_Color.C_B=52;
		break;
	case 237:
		m_Layer_Color.C_R=104;m_Layer_Color.C_G=69;m_Layer_Color.C_B=86;
		break;
	case 238:
		m_Layer_Color.C_R=79;m_Layer_Color.C_G=0;m_Layer_Color.C_B=39;
		break;
	case 239:
		m_Layer_Color.C_R=79;m_Layer_Color.C_G=53;m_Layer_Color.C_B=66;
		break;
	case 240:
		m_Layer_Color.C_R=255;m_Layer_Color.C_G=0;m_Layer_Color.C_B=63;
		break;
	case 241:
		m_Layer_Color.C_R=255;m_Layer_Color.C_G=170;m_Layer_Color.C_B=191;
		break;
	case 242:
		m_Layer_Color.C_R=189;m_Layer_Color.C_G=0;m_Layer_Color.C_B=46;
		break;
	case 243:
		m_Layer_Color.C_R=189;m_Layer_Color.C_G=126;m_Layer_Color.C_B=141;
		break;
	case 244:
		m_Layer_Color.C_R=129;m_Layer_Color.C_G=0;m_Layer_Color.C_B=31;
		break;
	case 245:
		m_Layer_Color.C_R=129;m_Layer_Color.C_G=86;m_Layer_Color.C_B=96;
		break;
	case 246:
		m_Layer_Color.C_R=104;m_Layer_Color.C_G=0;m_Layer_Color.C_B=25;
		break;
	case 247:
		m_Layer_Color.C_R=104;m_Layer_Color.C_G=69;m_Layer_Color.C_B=78;
		break;
	case 248:
		m_Layer_Color.C_R=79;m_Layer_Color.C_G=0;m_Layer_Color.C_B=19;
		break;
	case 249:
		m_Layer_Color.C_R=79;m_Layer_Color.C_G=53;m_Layer_Color.C_B=59;
		break;
	case 250:
		m_Layer_Color.C_R=51;m_Layer_Color.C_G=51;m_Layer_Color.C_B=51;
		break;
	case 251:
		m_Layer_Color.C_R=80;m_Layer_Color.C_G=80;m_Layer_Color.C_B=80;
		break;
	case 252:
		m_Layer_Color.C_R=105;m_Layer_Color.C_G=105;m_Layer_Color.C_B=105;
		break;
	case 253:
		m_Layer_Color.C_R=130;m_Layer_Color.C_G=130;m_Layer_Color.C_B=130;
		break;
	case 254:
		m_Layer_Color.C_R=190;m_Layer_Color.C_G=190;m_Layer_Color.C_B=190;
		break;
	case 255:
		m_Layer_Color.C_R=255;m_Layer_Color.C_G=255;m_Layer_Color.C_B=255;
		break;
	}

	return 0;
}


void CFileEntityViewer::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	
	if(bool_orgview == TRUE)
		menu.CheckMenuItem(IDM_ORGVIEW , MF_CHECKED);
	else
		menu.CheckMenuItem(IDM_ORGVIEW , MF_UNCHECKED);

	if(bool_absview == TRUE)
		menu.CheckMenuItem(IDM_ABSVIEW , MF_CHECKED);
	else
		menu.CheckMenuItem(IDM_ABSVIEW , MF_UNCHECKED);

	if(bool_absmove == TRUE)
		menu.CheckMenuItem(IDM_ABSMOVE , MF_CHECKED);
	else
		menu.CheckMenuItem(IDM_ABSMOVE , MF_UNCHECKED);

	if(bool_sector == TRUE)
		menu.CheckMenuItem(IDM_SECTOR , MF_CHECKED);
	else
		menu.CheckMenuItem(IDM_SECTOR , MF_UNCHECKED);

	if(bool_change_sector == TRUE)
		menu.CheckMenuItem(IDM_CHANGE_SECTOR , MF_CHECKED);
	else
		menu.CheckMenuItem(IDM_CHANGE_SECTOR , MF_UNCHECKED);

	CPoint p;
	GetCursorPos(&p); 
	menu.TrackPopupMenu(TPM_LEFTALIGN, p.x,p.y, this);
	


	CWnd::OnRButtonDown(nFlags, point);
}


void CFileEntityViewer::OnAbsmove() 
{
	SetAbsMove();
}

void CFileEntityViewer::OnAbsview() 
{
	SetViewAbs();
}

void CFileEntityViewer::OnOrgview() 
{
	SetViewORG();
}

void CFileEntityViewer::OnSector() 
{
	SetViewSector();
}

void CFileEntityViewer::OnChangeSector() 
{
	SetModifySector();
}

void CFileEntityViewer::OnViewArrow() 
{
	SetViewArrow();
}

void CFileEntityViewer::OnLButtonDown(UINT nFlags, CPoint point) 
{
	SetFocus();

	double px,py;

	px=(double)((point.x-m_right)/(m_zoom*m_dxfzoom));
	py=(double)((View_Y-m_updown-point.y)/(m_zoom*m_dxfzoom));
	//py=(double)((point.y-m_updown)/(m_zoom*m_dxfzoom));
	CString str;

	if(bool_absview==TRUE)
	{
		str.Format(L"%0.3f",px);
		m_viewer_abs_x=str;
		str.Format(L"%0.3f",py);
		m_viewer_abs_y=str;
		if(bool_absmove==TRUE)
		{
			Posx=(m_dFileStartX+px);
			Posy=(m_dFileStartY+py);
		}
	}
	else if(m_bOrderModify == TRUE)
	{
		if(m_nInputNum[pData->m_nSelLayer] < pData->m_pLayer[pData->m_nSelLayer].iRealBlockCount)
		{
			int nSectNum = FindSectorNum(px, py);
			if(pData->m_pLayer[pData->m_nSelLayer].pCutBlock[nSectNum].lOrderNumber == _SECT_INPUT )
			{
				pData->m_pLayer[pData->m_nSelLayer].pBlockOrder[m_nInputNum[pData->m_nSelLayer]] = nSectNum;
				pData->m_pLayer[pData->m_nSelLayer].pCutBlock[nSectNum].lOrderNumber = m_nInputNum[pData->m_nSelLayer];

				m_nInputNum[pData->m_nSelLayer]++;
			}
		}
	}

	Mini_Rectx=px;
	Mini_Recty=py;
	View_Draw_Line();			
	CWnd::OnLButtonDown(nFlags, point);
}

void CFileEntityViewer::OnMouseMove(UINT nFlags, CPoint point) 
{
	// todo: add your message handler code here and/or call default
	Mouse_MoveX = (point.x-m_right)/(m_dxfzoom*m_zoom);
	Mouse_MoveY = (View_Y-point.y-m_updown)/(m_dxfzoom*m_zoom);

	if((nFlags & MK_MBUTTON)==MK_MBUTTON)
	{
		if(point.x>0 && point.x <View_X && 
			point.y>0 && point.y<View_Y)
		{
			m_right=m_right+point.x-m_msx;
			m_updown=m_updown+(m_msy-point.y);

			View_Draw_Line();
			m_msx=point.x;
			m_msy=point.y;
		}
	}

	if(bool_change_sector == TRUE)
	{
		int i;

		if(((nFlags & MK_LBUTTON)==MK_LBUTTON)&& (Sel_MoveLine != -1))
		{
			if(point.x>0 && point.x <View_X && 
				point.y>0 && point.y<View_Y)
			{
				if( (m_Sector_Point[Sel_MoveLine].Row_Line == 0 )||
					(m_Sector_Point[Sel_MoveLine].Row_Line == pData->m_SectLine.RowCnt - 1 )||
					(m_Sector_Point[Sel_MoveLine].Col_Line == 0 )||
					(m_Sector_Point[Sel_MoveLine].Col_Line == pData->m_SectLine.ColCnt - 1 ))
					return;

				ClickFlag = TRUE;

				double mouse_MoveX = (point.x-m_mlx)/(m_dxfzoom*m_zoom);
				double mouse_MoveY = (point.y-m_mly)/(m_dxfzoom*m_zoom)*(-1);

				pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line].sx=
					pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line].sx + (mouse_MoveX);
				pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line].ex=
					pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line].ex + (mouse_MoveX);

				pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line].sy=
					pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line].sy + (mouse_MoveY);
				pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line].ey=
					pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line].ey + (mouse_MoveY);

				//라인 삭제 부분
				if(Change_um(pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line+1].sx - 
					pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line].sx)<1)
				{
					if((m_Sector_Point[Sel_MoveLine].Row_Line == pData->m_SectLine.RowCnt-2) &&
						(Change_um(pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line+1].sx - 
						pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line-1].sx)<Change_um(pData->m_iBlockSize)))
					{
						m_Sector_Point[Sel_MoveLine].Mouse_Flag = TRUE;
						Delete_Sector_Line(_ROW_LINE);
						View_Draw_Line();
						ClickFlag = FALSE;
						Sel_MoveLine = -1;

						return;
					}
				}

				if(Change_um(pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line+1].sy - 
					pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line].sy)<1)
				{
					if((m_Sector_Point[Sel_MoveLine].Col_Line == pData->m_SectLine.ColCnt-2) &&
						(Change_um(pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line].sy - 
						pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line-1].sy)<Change_um(pData->m_iBlockSize)))
					{					
						m_Sector_Point[Sel_MoveLine].Mouse_Flag = TRUE;
						Delete_Sector_Line(_COL_LINE);
						View_Draw_Line();
						ClickFlag = FALSE;
						Sel_MoveLine = -1;
						return;
					}
				}

				//뒤라인과 간격이 맥스 보다 크거나 0보다 작은 경우
				if((Change_um(pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line+1].sx - 
					pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line].sx)<1)||
					(Change_um(pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line+1].sx - 
					pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line].sx)>Change_um(pData->m_iBlockSize)))
				{
					pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line].sx -= (mouse_MoveX);
					pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line].ex -= (mouse_MoveX);
				}

				if((Change_um(pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line+1].sy - 
					pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line].sy)<1)||
					(Change_um(pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line+1].sy - 
					pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line].sy)>Change_um(pData->m_iBlockSize)))
				{
					pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line].sy -= (mouse_MoveY);
					pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line].ey -= (mouse_MoveY);
				}


				//앞라인과 간격이 맥스 보다 크거나 0보다 작은 경우
				if((Change_um(pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line].sx - 
					pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line-1].sx)>Change_um(pData->m_iBlockSize))||
					(Change_um(pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line].sx - 
					pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line-1].sx)<1))
				{
					pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line].sx -= (mouse_MoveX);
					pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line].ex -= (mouse_MoveX);
				}

				if((Change_um(pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line].sy - 
					pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line-1].sy)>Change_um(pData->m_iBlockSize))||
					(Change_um(pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line].sy - 
					pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line-1].sy) < 1))
				{
					pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line].sy -= (mouse_MoveY);
					pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line].ey -= (mouse_MoveY);
				}

				//뒤라인과 간격이 맥스 보다 크거나 0보다 작은 경우
				if((Change_um(pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line+1].sx - 
					pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line].sx) < 1 ||
					(pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line+1].sx - 
					pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line].sx)>Change_um(pData->m_iBlockSize)))
				{
					pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line].sx -= (mouse_MoveX);
					pData->m_SectLine.Row[m_Sector_Point[Sel_MoveLine].Row_Line].ex -= (mouse_MoveX);
				}

				if((Change_um(pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line+1].sy - 
					pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line].sy)<1)||
					((pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line+1].sy - 
					pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line].sy)>Change_um(pData->m_iBlockSize)))
				{
					pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line].sy -= (mouse_MoveY);
					pData->m_SectLine.Col[m_Sector_Point[Sel_MoveLine].Col_Line].ey -= (mouse_MoveY);
				}

				Cal_Sector_Point();
				m_Sector_Point[Sel_MoveLine].Mouse_Flag = TRUE;
				View_Draw_Line();
				m_mlx=point.x;
				m_mly=point.y;
			}
			ClickFlag = FALSE;
		}
		else //Sector change & no click move
		{
			Cal_Sector_Point();

			double dRtWidth = (10.0)/(m_dxfzoom*m_zoom);

			for(i=0;i<m_Sector_Point_Cnt;i++)
			{
				double x = (Mouse_MoveX);
				double y = (Mouse_MoveY);

				if(   Change_um(x) >= Change_um(m_Sector_Point[i].posX-dRtWidth) && 
					  Change_um(x) <= Change_um(m_Sector_Point[i].posX+dRtWidth) &&
					  Change_um(y) >= Change_um(m_Sector_Point[i].posY-dRtWidth) && 
					  Change_um(y) <= Change_um(m_Sector_Point[i].posY+dRtWidth) )
				{
					m_Sector_Point[i].Mouse_Flag = TRUE;
					Sel_MoveLine = i;
					break;
				}
				else //Outside of Area
				{
					m_Sector_Point[i].Mouse_Flag = FALSE;
					Sel_MoveLine = -1;

				}
			}
			View_Draw_Line();
			m_msx=point.x;
			m_msy=point.y;	
			m_mlx=point.x;
			m_mly=point.y;	
		}
	}

	m_msx=point.x;
	m_msy=point.y;	

	CWnd::OnMouseMove(nFlags, point);
}

BOOL CFileEntityViewer::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	MouseWheel(nFlags, zDelta, pt);

	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

int CFileEntityViewer::Change_int(double val)
{
// 	if (val >= 0)
// 		return (int) (val+0.5);
// 	return (int) (val-0.5);

	return (int) val;
}




void CFileEntityViewer::MouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if(m_zoom <= 0 || m_zoom > 1000)
		return;

	ScreenToClient(&pt);

	double dDistanceX = pt.x - m_right;
	double dDistanceY = View_Y - pt.y - m_updown;

	double ago_zoom = m_zoom;

	m_zoom = (zDelta > 0) ? m_zoom * 1.2 : m_zoom / 1.2;

	if(m_zoom > 1000) m_zoom = 1000;

	//CurDistance - After Zoom IN/OUT Distance
	dDistanceX = dDistanceX - dDistanceX * m_zoom / ago_zoom;
	dDistanceY = dDistanceY - dDistanceY * m_zoom / ago_zoom;

	m_right += dDistanceX;
	m_updown += dDistanceY;

	View_Draw_Line();
}

void CFileEntityViewer::Close_FileView()
{
	if(bool_absmove==TRUE)
		KillTimer(1);
}


void CFileEntityViewer::OnTimer(UINT nIDEvent) 
{
	switch(nIDEvent)
	{
	case 1:
		{
			if(bool_absmove2==FALSE)
				break;
			else
			{
				if(bool_absmove2==TRUE)	
				{ 
					   
					abs_move_posx = m_dCurrentX - m_dFileStartX; 
					abs_move_posy = m_dCurrentY - m_dFileStartY; 
					View_Draw_Line();
				}		
				if(abs(int(abs_move_posx-Posx))>3 && abs(int(abs_move_posy-Posy))>3)
					break;
			}
		}
	}

	CWnd::OnTimer(nIDEvent);
}

int CFileEntityViewer::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	menu.CreatePopupMenu();
	menu.AppendMenu(MF_STRING, IDM_ORGVIEW, L"원점보기");
	menu.AppendMenu(MF_STRING, IDM_ABSVIEW, L"좌표보기");
	menu.AppendMenu(MF_STRING, IDM_VIEWARROW, L"화살표보기");

	//menu.AppendMenu(MF_STRING, IDM_ABSMOVE, L"좌표이동");
	//menu.AppendMenu(MF_STRING, IDM_SECTOR,  L"섹터보기");
	//menu.AppendMenu(MF_STRING, IDM_CHANGE_SECTOR, L"섹터수정");
	// TODO: Add your specialized creation code here

	static PIXELFORMATDESCRIPTOR pfd = 
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER   |
		PFD_TYPE_RGBA,
		24,
		0,0,0,0,0,0,
		0,0,
		0,0,0,0,0,
		32,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0,0,0
	};

	m_hDC = ::GetDC(m_hWnd);
	int nPixelFormat = ChoosePixelFormat(m_hDC, &pfd);
	VERIFY(SetPixelFormat(m_hDC, nPixelFormat, &pfd));
	m_hRC = wglCreateContext(m_hDC);
	VERIFY(wglMakeCurrent(m_hDC, m_hRC));
	wglMakeCurrent(NULL, NULL);

	return 0;
}

void CFileEntityViewer::View_Draw_Ruler(Graphics *pDC)
{
	CRect rect;
	int lr=255,lg=255,lb=255;
	int i;
	
	Pen pen(Color(255,255,0), 1);

	Font fntGulim(L"굴림체", 15, FontStyleRegular, UnitPixel);
	SolidBrush whiteBrush( Color(255, 255, 255));

	double line_start_x;
	double line_start_y;
	double line_end_x;
	double line_end_y;
	line_start_x=(double)((0-m_right)/(m_zoom*m_dxfzoom));
	line_start_y=(double)((View_Y+m_updown-0)/(m_zoom*m_dxfzoom));
	line_end_x=(double)((View_X-m_right)/(m_zoom*m_dxfzoom));
	line_end_y=(double)((View_Y+m_updown-View_Y)/(m_zoom*m_dxfzoom));

	int ruler_length = Change_int(line_end_x-line_start_x);
	CString ruler_num;

	for(i=Change_int(line_start_x-50);i<Change_int(line_end_x+50);i++)
	{
		if(ruler_length>200 && ruler_length<1000)
		{
			if(i%5==0)
			{
				double l;
				l=double(i*m_zoom*m_dxfzoom+m_right);

				if (i%30==0 && ruler_length<400)
				{
					ruler_num.Format(L"%d",i);
					pDC->DrawString(ruler_num, ruler_num.GetLength(), &fntGulim, PointF(l-8,View_Y-25), &whiteBrush );
				}

				if(i%10 == 0)
				{
					pDC->DrawLine(&pen, float(l), float(View_Y), float(l), float(View_Y-10));
				}
				else
				{
					pDC->DrawLine(&pen, float(l), float(View_Y), float(l), float(View_Y-5));
				}
			}

		}
		else if(ruler_length<200)
		{
			double l;
			l=double(i*m_zoom*m_dxfzoom+m_right);
			if (i%10==0 && ruler_length<400)
			{
				ruler_num.Format(L"%d",i);
				pDC->DrawString(ruler_num, ruler_num.GetLength(), &fntGulim, PointF(l-8,View_Y-25), &whiteBrush );
			}
			if(i%10 == 0)
			{
				pDC->DrawLine(&pen, float(l), float(View_Y), float(l), float(View_Y-10));
			}
			else if(i%5 == 0)
			{
				pDC->DrawLine(&pen, float(l), float(View_Y), float(l), float(View_Y-8));
			}
			else
			{
				pDC->DrawLine(&pen, float(l), float(View_Y), float(l), float(View_Y-5));
			}

		}
	}
	for(i=Change_int(line_end_y-50) ;i<Change_int(line_start_y+50);i++)
	{
		if(ruler_length>200 && ruler_length<1000)
		{
			if(i%5==0)
			{
				double l;
				l=double(View_Y-i*m_zoom*m_dxfzoom+m_updown);
				if (i%30==0 && ruler_length<400)
				{
					ruler_num.Format(L"%d",i);
					pDC->DrawString(ruler_num, ruler_num.GetLength(), &fntGulim, PointF(15,l-8), &whiteBrush );
				}

				if(i%10 == 0)
				{
					pDC->DrawLine(&pen, float(0), float(l), float(10), float(l));
				}
				else
				{
					pDC->DrawLine(&pen, float(0), float(l), float(5), float(l));
				}
			}
		}
		else if(ruler_length<200 )
		{
			double l;
			l=double(View_Y-i*m_zoom*m_dxfzoom+m_updown);

			if (i%10==0 && ruler_length<400)
			{
				ruler_num.Format(L"%d",i);

				pDC->DrawString(ruler_num, ruler_num.GetLength(), &fntGulim, PointF(15,l-8), &whiteBrush );
			}

			if(i%10 == 0)
			{
				pDC->DrawLine(&pen, float(0), float(l), float(10), float(l));
			}
			else if(i%5 == 0)
			{
				pDC->DrawLine(&pen, float(0), float(l), float(8), float(l));
			}
			else
			{
				pDC->DrawLine(&pen, float(0), float(l), float(5), float(l));
			}

		}
	}

}

void CFileEntityViewer::View_Draw_AbsView(Graphics *pDC)
{
	//CClientDC dc(this); // device context for painting

	double temp_posx,temp_posy;

// 	Pen pen( Color (255, 0, 0), 1 );
// 	Font fntGulim(L"굴림체", 15, FontStyleRegular, UnitPixel);
// 	SolidBrush whiteBrush( Color(255, 255, 255));

	temp_posx=_ttof(m_viewer_abs_x)*m_zoom*m_dxfzoom+m_right;
	temp_posy=_ttof(m_viewer_abs_y)*m_zoom*m_dxfzoom+m_updown;

// 	pDC->DrawLine(&pen, float(temp_posx-5), float(temp_posy-5), float(temp_posx+5), float(temp_posy+5));
// 	pDC->DrawLine(&pen, float(temp_posx+5), float(temp_posy-5), float(temp_posx-5), float(temp_posy+5));

	glBegin(GL_LINE_STRIP);
	glVertex2f(temp_posx-5 , temp_posy-5);	
	glVertex2f(temp_posx+5 , temp_posy+5);	
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex2f(temp_posx+5 , temp_posy-5);
	glVertex2f(temp_posx-5 , temp_posy+5);
	glEnd();
	

 	CString temp = L"  X: "+m_viewer_abs_x+L"  Y: "+m_viewer_abs_y;
	DrawText(550, 10, temp, GLUT_BITMAP_9_BY_15, 255, 255, 255);
	//pDC->DrawString(temp, temp.GetLength(), &fntGulim, PointF(400,20), &whiteBrush );
}

void CFileEntityViewer::View_Draw_AbsMove(Graphics *pDC)
{
	Pen pen( Color (255, 0, 0), 1 );
	
	CPoint mini1,mini2; //파일뷰 화면에서의 비전화면 부분 
	mini1.x=(long)((Mini_Rectx-2.5)*m_zoom*m_dxfzoom+m_right+0.5);
	mini2.x=(long)((Mini_Rectx+2.5)*m_zoom*m_dxfzoom+m_right+0.5);
	mini1.y=(long)(View_Y-(Mini_Recty+2.5)*m_zoom*m_dxfzoom+m_updown+0.5);
	mini2.y=(long)(View_Y-(Mini_Recty-2.5)*m_zoom*m_dxfzoom+m_updown+0.5);

// 	pDC->MoveTo(mini1.x,mini1.y);
// 	pDC->LineTo(mini2.x,mini1.y);
// 	pDC->LineTo(mini2.x,mini2.y);
// 	pDC->LineTo(mini1.x,mini2.y);
// 	pDC->LineTo(mini1.x,mini1.y);


// 	pDC->SetTextAlign(TA_LEFT|TA_TOP);
// 	pDC->SetTextColor(RGB(255,255,0));
// 	pDC->SetBkColor(RGB(50,51,51));
// 	CString temp = L"  X: "+m_viewer_abs_x+L"  Y: "+m_viewer_abs_y;
// 	pDC->TextOut(400,20,temp);

	if(bool_absmove2==TRUE)
	{
		mini1.x=(long)((abs_move_posx-2.5)*m_zoom*m_dxfzoom+m_right+0.5);
		mini2.x=(long)((abs_move_posx+2.5)*m_zoom*m_dxfzoom+m_right+0.5);
		mini1.y=(long)(View_Y-(abs_move_posy+2.5)*m_zoom*m_dxfzoom+m_updown+0.5);
		mini2.y=(long)(View_Y-(abs_move_posy-2.5)*m_zoom*m_dxfzoom+m_updown+0.5);

// 		pDC->MoveTo(mini1.x,mini1.y);
// 		pDC->LineTo(mini2.x,mini1.y);
// 		pDC->LineTo(mini2.x,mini2.y);
// 		pDC->LineTo(mini1.x,mini2.y);
// 		pDC->LineTo(mini1.x,mini1.y);
	}

}

void CFileEntityViewer::View_Draw_Sector(Graphics *pDC)
{
	//CClientDC dc(this); // device context for painting
	int i,j;

	double a1,a2,a3,a4;

	//Pen pen( Color (255, 255, 0), 1 );
	glColor3f(1.0f, 1.0f, 0.0f);

	for( i=0;i<pData->m_SectLine.ColCnt-1;i++)
	{
		for( j=0;j<pData->m_SectLine.RowCnt-1;j++)
		{
			a1=(pData->m_SectLine.Row[j].sx);
			a2=(pData->m_SectLine.Col[i].sy);
			a3=(pData->m_SectLine.Row[j+1].ex);
			a4=(pData->m_SectLine.Col[i+1].ey);

			a1=a1*m_dxfzoom*m_zoom+m_right;
			a2=a2*m_dxfzoom*m_zoom+m_updown;
			a3=a3*m_dxfzoom*m_zoom+m_right;
			a4=a4*m_dxfzoom*m_zoom+m_updown;


			glBegin(GL_LINE_STRIP);
			glVertex2f(a1-5 , a2);	
			glVertex2f(a3+5 , a2);
			glEnd();

			glBegin(GL_LINE_STRIP);
			glVertex2f(a3, a2-5);	
			glVertex2f(a3, a4+5);
			glEnd();

			glBegin(GL_LINE_STRIP);
			glVertex2f(a3+5 , a4);	
			glVertex2f(a1-5 , a4);
			glEnd();

			glBegin(GL_LINE_STRIP);
			glVertex2f(a1, a4+5);	
			glVertex2f(a1, a2-5);
			glEnd();

// 			pDC->DrawLine(&pen, float(a1-5), float(a2), float(a3+5), float(a2));
// 			pDC->DrawLine(&pen, float(a3), float(a2+5), float(a3), float(a4-5));
// 			pDC->DrawLine(&pen, float(a3+5), float(a4), float(a1-5), float(a4));
// 			pDC->DrawLine(&pen, float(a1), float(a4-5), float(a1), float(a2+5));
		}
	}
}


void CFileEntityViewer::View_Draw_SectorLine(Graphics *pDC)
{
	//CClientDC dc(this); // device context for painting
	int i;

// 	Pen pen( Color (255, 255, 0), 1 );
// 	SolidBrush redBrush( Color(255, 0, 0) );
// 	SolidBrush yelloBrush( Color(255, 255, 0) );

	double a1,a2,a3,a4;

	glColor3f(1.0f, 1.0f, 0.0f);

	
	for( i=0;i<pData->m_SectLine.RowCnt;i++)
	{
		a1=(pData->m_SectLine.Row[i].sx);
		a2=(pData->m_SectLine.Row[i].sy);
		a3=(pData->m_SectLine.Row[i].ex);
		a4=(pData->m_SectLine.Row[i].ey);
		a1=a1*m_dxfzoom*m_zoom+m_right;
		a2=a2*m_dxfzoom*m_zoom+m_updown;
		a3=a3*m_dxfzoom*m_zoom+m_right;
		a4=a4*m_dxfzoom*m_zoom+m_updown;

		glBegin(GL_LINE_STRIP);
		glVertex2f(a1, a2);	
		glVertex2f(a3, a4);
		glEnd();
	}
	
	
	for( i=0;i<pData->m_SectLine.ColCnt;i++)
	{
		a1=(pData->m_SectLine.Col[i].sx);
		a2=(pData->m_SectLine.Col[i].sy);
		a3=(pData->m_SectLine.Col[i].ex);
		a4=(pData->m_SectLine.Col[i].ey);
		a1=a1*m_dxfzoom*m_zoom+m_right;
		a2=a2*m_dxfzoom*m_zoom+m_updown;
		a3=a3*m_dxfzoom*m_zoom+m_right;
		a4=a4*m_dxfzoom*m_zoom+m_updown;

		glBegin(GL_LINE_STRIP);
		glVertex2f(a1, a2);	
		glVertex2f(a3, a4);
		glEnd();
	}

	for(i=0;i<m_Sector_Point_Cnt;i++)
	{
		a1=(m_Sector_Point[i].posX);
		a2=(m_Sector_Point[i].posY);

		a1=a1*m_dxfzoom*m_zoom+m_right;
		a2=a2*m_dxfzoom*m_zoom+m_updown;

		//if(m_Sector_Point[i].Mouse_Flag == FALSE)
		//	glColor3f(255/255.0f,255/255.0f,0/255.0f); //그릴 색을 정함
		if(m_Sector_Point[i].Mouse_Flag == TRUE){
			glColor3f(255/255.0f,0/255.0f,0/255.0f); //그릴 색을 정함
			glRectf(a1-4, a2-4, a1+4, a2+4);
		}
	}
}


void CFileEntityViewer::View_Draw_SectorOrder(Graphics *pDC)
{
	if(pData->m_nSelLayer == -1)
		return;

	Pen penOrder(Color(255,255,0),2);
	
	SolidBrush brushGrey( Color(50, 50, 50) );
	SolidBrush brushWhite( Color(255, 255, 255) );
	Font fntGulim(L"굴림체", 13, FontStyleBold, UnitPixel);
	StringFormat strFrmt;
	strFrmt.SetAlignment(StringAlignmentCenter);
	strFrmt.SetLineAlignment(StringAlignmentCenter);
	
	double a1,a2,a3,a4;	
	CString str;

	int i, j, skip = 0, nNum = 0;
	for( i=0;i<pData->m_SectLine.ColCnt-1;i++)
	{
		for( j=0;j<pData->m_SectLine.RowCnt-1;j++)
		{
			a1=(pData->m_SectLine.Row[j].sx);
			a2=(pData->m_SectLine.Col[i].sy);
			a3=(pData->m_SectLine.Row[j+1].ex);
			a4=(pData->m_SectLine.Col[i+1].ey);

			a1=a1*m_dxfzoom*m_zoom+m_right;
			a2=a2*m_dxfzoom*m_zoom+m_updown;
			a3=a3*m_dxfzoom*m_zoom+m_right;
			a4=a4*m_dxfzoom*m_zoom+m_updown;

			double dSx = (a1 < a3) ? a1 : a3;
			double dSy = (a2 < a4) ? a2 : a4;
			double dEx = (a1 < a3) ? a3 : a1;
			double dEy = (a2 < a4) ? a4 : a2;

			double dStrSY, dStrW, dStrH;
			dStrSY = View_Y - dEy;
			dStrW = dEx - dSx;
			dStrH = dEy - dSy;

			if( pData->m_pLayer[pData->m_nSelLayer].pCutBlock[nNum].lOrderNumber != _SECT_EMPTY )
 			{
				glEnable( GL_BLEND );
				glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  				if( pData->m_pLayer[pData->m_nSelLayer].pCutBlock[nNum].lOrderNumber == _SECT_INPUT)
  				{
					glColor4f(255.0f,128/255.0f, 0/255.0f, 180/255.0f); //그릴 색을 정함
					glRectf(dSx+2, dSy+2, dEx-2, dEy-2); //사각형
  				}
  				else
 				{
					glColor4f(95/255.0f,157/255.0f,255/255.0f,180/255.0f); //그릴 색을 정함
					glRectf(dSx+2, dSy+2, dEx-2, dEy-2); //사각형
// 					pDC->DrawString(str, str.GetLength(), &fntGulim, RectF( float(dSx-1), float(dSy+3), float(dEx-dSx), float(dEy-dSy)), &strFrmt, &brushGrey);
// 					pDC->DrawString(str, str.GetLength(), &fntGulim, RectF( float(dSx-3), float(dSy+1), float(dEx-dSx), float(dEy-dSy)), &strFrmt, &brushWhite);

					str.Format(L"%d", pData->m_pLayer[pData->m_nSelLayer].pCutBlock[nNum].lOrderNumber+1);
					
					short nFontGapX, nFontGapY;
					float nFontShadow = 1.3;
					if( (dEx - dSx) > 24 )
					{
						nFontGapX = str.GetLength() * 5;
						nFontGapY = 5;
						
						DrawText((dSx+dEx)/2-nFontGapX, (dSy+dEy)/2-nFontGapY, str, GLUT_BITMAP_HELVETICA_18, 1, 1, 1);
					}
					else if( (dEx - dSx) > 14 )
					{
						nFontGapX = str.GetLength() * 3;
						nFontGapY = 3;
						
						DrawText((dSx+dEx)/2-nFontGapX, (dSy+dEy)/2-nFontGapY, str, GLUT_BITMAP_HELVETICA_12, 1, 1, 1);
					}
					else
					{					
						str = L".";
						DrawText((dSx+dEx)/2, (dSy+dEy)/2, str, GLUT_BITMAP_HELVETICA_10, 1, 1, 1);
					}
 				}
				
				glLineWidth(1.5f);
				glDisable( GL_BLEND );
				glColor3f(95/255.0f,157/255.0f,255/255.0f); //그릴 색을 정함
				//glColor3f(255/255.0f,255/255.0f,0/255.0f); //그릴 색을 정함
				glBegin(GL_LINE_STRIP);
				glVertex2f(dSx, dSy);	
				glVertex2f(dEx, dSy);	
				glVertex2f(dEx, dEy);	
				glVertex2f(dSx, dEy);	
				glVertex2f(dSx, dSy);
				glEnd();
			}
 			nNum++;
		}
	}
	
}


double CFileEntityViewer::Round(double number)
{
	number =  floor(1000000.*(number + 0.0000005))/1000000.;
	return number;
}



void CFileEntityViewer::Cal_Sector_Point()
{
	m_Sector_Point_Cnt = (pData->m_SectLine.ColCnt)*(pData->m_SectLine.RowCnt);
	if(m_Sector_Point!=NULL)
	{
		delete[] m_Sector_Point;
		m_Sector_Point = NULL;
	}
	m_Sector_Point = new SECTOR_POINT[m_Sector_Point_Cnt];
	int i,j;
	int point_cnt = 0;

	for(i=0;i<pData->m_SectLine.ColCnt;i++)
	{
		for(j=0;j<pData->m_SectLine.RowCnt;j++)
		{
			m_Sector_Point[point_cnt].Col_Line = i;
			m_Sector_Point[point_cnt].Row_Line = j;
			m_Sector_Point[point_cnt].posX = pData->m_SectLine.Row[j].sx;
			m_Sector_Point[point_cnt].posY = pData->m_SectLine.Col[i].sy;
			//m_Sector_Point[point_cnt].Mouse_Flag = FALSE;
			point_cnt++;
		}
	}
}

void CFileEntityViewer::Delete_Sector_Line(int line)
{
	pData->DeleteSectorLine(line);
// 	if(line == _ROW_LINE)
// 	{
		
// 		m_Sector_Line_Row[m_Sector_Line_Row_Cnt-1].sx = m_Sector_Line_Row[m_Sector_Line_Row_Cnt].sx;
// 		m_Sector_Line_Row[m_Sector_Line_Row_Cnt-1].sy = m_Sector_Line_Row[m_Sector_Line_Row_Cnt].sy;
// 		m_Sector_Line_Row[m_Sector_Line_Row_Cnt-1].ex = m_Sector_Line_Row[m_Sector_Line_Row_Cnt].ex;
// 		m_Sector_Line_Row[m_Sector_Line_Row_Cnt-1].ey = m_Sector_Line_Row[m_Sector_Line_Row_Cnt].ey;
// 		m_Sector_Line_Row_Cnt--;

// 	}
// 	else
// 	{
// 		m_Sector_Line_Col[m_Sector_Line_Col_Cnt-1].sx = m_Sector_Line_Col[m_Sector_Line_Col_Cnt].sx;
// 		m_Sector_Line_Col[m_Sector_Line_Col_Cnt-1].sy = m_Sector_Line_Col[m_Sector_Line_Col_Cnt].sy;
// 		m_Sector_Line_Col[m_Sector_Line_Col_Cnt-1].ex = m_Sector_Line_Col[m_Sector_Line_Col_Cnt].ex;
// 		m_Sector_Line_Col[m_Sector_Line_Col_Cnt-1].ey = m_Sector_Line_Col[m_Sector_Line_Col_Cnt].ey;
// 		m_Sector_Line_Col_Cnt--;
/*	}*/

	Cal_Sector_Point();
	View_Draw_Line();

}

void CFileEntityViewer::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if(Sel_MoveLine != -1)
	{
		
		Cal_Sector_Point();
		View_Draw_Line();
	}

	CWnd::OnLButtonUp(nFlags, point);
}

void CFileEntityViewer::Insert_Row_Line()
{
	//if(pData->m_b240Mode)
//		return;

	pData->AddSectorLine(_ROW_LINE);

	Cal_Sector_Point();
	View_Draw_Line();
}

void CFileEntityViewer::Insert_Col_Line()
{
	//if(pData->m_b240Mode)
	//	return;

	pData->AddSectorLine(_COL_LINE);

	Cal_Sector_Point();
	View_Draw_Line();
}


void CFileEntityViewer::OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// This feature requires Windows Vista or greater.
	// The symbol _WIN32_WINNT must be >= 0x0600.
	// TODO: Add your message handler code here and/or call default

	CWnd::OnMouseHWheel(nFlags, zDelta, pt);
}


BOOL CFileEntityViewer::InitViewer(CWnd* pParent)
{
	CRect rcImage, rcClient, rt;

	pParent->GetClientRect(&rt);

	BOOL bResult = Create(NULL, NULL, WS_VISIBLE|WS_CHILD, rt, pParent, 0);

	View_X=rt.right-rt.left;
	View_Y=rt.bottom-rt.top;

	CRect rect;
	GetClientRect(rect);

	m_nUnit	= 1;
	m_nWidth = rt.Width()+1;
	m_nHeight = rt.Height()+1;

	m_bUse = new BOOL*[m_nWidth];
	for(int width=0; width<m_nWidth;width++)
	{
		m_bUse[width] = new BOOL[m_nHeight];
		memset(m_bUse[width], FALSE, sizeof(m_bUse[width]) * m_nHeight);
	}

	return bResult;

	return 0;
}

void CFileEntityViewer::SetLayerViewer(int nLayerNum, BOOL bFlag)
{
	m_bViewLayer[nLayerNum] = bFlag;

	View_Draw_Line();
}




void CFileEntityViewer::GetCoord(float* fCoordX, float* fCoordY)
{
	*fCoordX = (float)Mouse_MoveX;
	*fCoordY = (float)Mouse_MoveY;
}

void CFileEntityViewer::SetClickMove(BOOL flag)
{
	bClickMove = flag;
}


void CFileEntityViewer::MakeFolders(void)
{
	CString TempStr;			/** TempStr local variable 01*/
	CString TempStr2;			/** TempStr2 local variable 02*/
	int TempLength;				/** TempLength local variable 03*/
	int TempNumber;				/** TempNumber local variable 04*/
	TCHAR folder[1000];			/** folder local variable 05*/

	////////////////// 블럭 실행 디렉토리 만들기 ///////////////////////////
	GetModuleFileName(AfxGetInstanceHandle(), folder, 1000);

	TempStr = folder;
	TempLength = TempStr.GetLength();
	TempStr.MakeReverse();
	TempNumber = TempStr.Find(_T("\\"),0);
	TempStr = TempStr.Right(TempLength-TempNumber-1);
	TempStr.MakeReverse();

	m_strCoordFolder	= TempStr+_T("\\Coord");
	m_strCommonFolder	= m_strCoordFolder+_T("\\Comm");
	m_strDxfFolder		= m_strCoordFolder+_T("\\Dxf");
	m_strBlockFolder	= m_strDxfFolder+_T("\\Block");
	m_strEntityFolder	= m_strDxfFolder+_T("\\Entity");

	CreateDirectory(m_strCoordFolder,	NULL);
	CreateDirectory(m_strCommonFolder,	NULL);
	CreateDirectory(m_strBlockFolder,	NULL);
	CreateDirectory(m_strDxfFolder,		NULL);
	CreateDirectory(m_strEntityFolder,	NULL);
}


void CFileEntityViewer::SetViewAbs()
{
	

	if(	bool_absview==FALSE)
		bool_absview=TRUE;
	else
	{
		bool_absview=FALSE;
		bool_absmove=FALSE;
		bool_absmove2=FALSE;
	}
	View_Draw_Line();
}


void CFileEntityViewer::SetAbsMove()
{
	

	if(	bool_absview==TRUE && bool_absmove==FALSE && m_bApply_flag==TRUE)
	{
		bool_absmove=TRUE;
		bool_absmove2=TRUE;
		SetTimer(1,500,NULL);
	}
	else if(bool_absview==TRUE && bool_absmove==FALSE && m_bApply_flag==FALSE)
	{
		AfxMessageBox(L"작업 조건이 적용되지 않았습니다");
		bool_absmove=FALSE;
		bool_absmove2=FALSE;
	}
	else
	{
		bool_absmove=FALSE;
		bool_absmove2=FALSE;
		KillTimer(1);
	}
	View_Draw_Line();
}


void CFileEntityViewer::SetViewSector()
{
	

	if(	bool_sector==FALSE)
		bool_sector=TRUE;
	else
		bool_sector=FALSE;

	View_Draw_Line();
}
void CFileEntityViewer::SetModifySector()
{

	if(	bool_change_sector==FALSE)
	{
		bool_change_sector=TRUE;
		Cal_Sector_Point();
	}
	else
	{
		bool_change_sector=FALSE;
	}
	View_Draw_Line();
}


void CFileEntityViewer::SetViewORG()
{
	

	if(	bool_orgview==FALSE)
		bool_orgview=TRUE;
	else
		bool_orgview=FALSE;

	View_Draw_Line();
}

void CFileEntityViewer::SetViewArrow()
{
	bViewArrow = !bViewArrow;

	View_Draw_Line();
}


void CFileEntityViewer::SetModifyORG(double fMoveX, double fMoveY)
{
	pData->ModifyORGCoord(fMoveX, fMoveY);
	Cal_Sector_Point();

	m_updown -= fMoveY*m_zoom*m_dxfzoom;
	m_right += fMoveX*m_zoom*m_dxfzoom;

	

	View_Draw_Line();
}


void CFileEntityViewer::SetVIewOrder(BOOL bFlag)
{
	

	m_bOrderView = bFlag;

	View_Draw_Line();
}


void CFileEntityViewer::SetModifyOrder(BOOL bFlag)
{
	m_bOrderModify = bFlag;
		
	if(bFlag == TRUE){
		m_nInputNum[pData->m_nSelLayer] = 0;
		pData->InputSectorOrder();
	}

	else{
		for(int i=0; i<pData->m_iLayerCount; i++)
		{
			m_nInputNum[i] = pData->m_pLayer[i].iRealBlockCount;
		}
		pData->InitSectorOrder(pData->m_nSelLayer);
	}
	View_Draw_Line();
}


void CFileEntityViewer::SetViewPattern()
{

}


void CFileEntityViewer::SetModifyPattern()
{

}

void CFileEntityViewer::SetFileLoad(BOOL bFlag)
{
	m_bFile_LoadIs = bFlag;
}

int CFileEntityViewer::Change_um(double val)
{
	//소수점 유효자리 6재짜리까지.
	return (int)(val * 1000000.0);
}


double CFileEntityViewer::Change_float(int num)
{
	//Change Um 복귀
	return num / 1000000.0;
}


int CFileEntityViewer::FindSectorNum(double fX, double fY)
{
	int nSectNum = -1;

	for(int i=0; i< pData->m_iMaxBlockNumber; i++)
	{
		if( (fX >= pData->m_pBlockPos[i].fLeft) && 
			(fY >= pData->m_pBlockPos[i].fBottom) && 
			(fX <= pData->m_pBlockPos[i].fRight) &&
			(fY <= pData->m_pBlockPos[i].fTop) )
		{
			nSectNum = i;
			break;
		}
	}

	return nSectNum;
}

int CFileEntityViewer::GetOrderModifyState(int nLayerNum)
{
	//배열로 설정하기

	for(int i=0; i<pData->m_iLayerCount; i++)
	{
		if(m_nInputNum[i] != pData->m_pLayer[i].iRealBlockCount)
			return -1;
	}	
	return 1;
}

void CFileEntityViewer::InitInputNumMEM()
{
	if(m_nInputNum != NULL)
	{
		delete[] m_nInputNum;
		m_nInputNum = NULL;
	}
	m_nInputNum = new int[pData->m_iLayerCount];

	for(int i=0; i<pData->m_iLayerCount; i++)
		m_nInputNum[i] = pData->m_pLayer[i].iRealBlockCount;
}

void CFileEntityViewer::SetSectorMove(double dMoveX, double dMoveY)
{
	double dTempX = dMoveX;
	double dTempY = dMoveY;
	
	dMoveX = dMoveX - pData->m_dSectorTotalMoveX;
	dMoveY = dMoveY - pData->m_dSectorTotalMoveY;

	pData->m_dSectorTotalMoveX = dTempX;
	pData->m_dSectorTotalMoveY = dTempY;
	
	for(int row = 0; row < pData->m_SectLine.RowCnt; row++)
	{
		pData->m_SectLine.Row[row].sx += (dMoveX);
		pData->m_SectLine.Row[row].ex += (dMoveX);
		pData->m_SectLine.Row[row].sy += (dMoveY);
		pData->m_SectLine.Row[row].ey += (dMoveY);

	}
	for(int col = 0; col < pData->m_SectLine.ColCnt; col++)
	{
		pData->m_SectLine.Col[col].sx += (dMoveX);
		pData->m_SectLine.Col[col].ex += (dMoveX);
		pData->m_SectLine.Col[col].sy += (dMoveY);
		pData->m_SectLine.Col[col].ey += (dMoveY);
	}

	Cal_Sector_Point();
	View_Draw_Line();
}

void CFileEntityViewer::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	VERIFY(wglMakeCurrent(m_hDC, m_hRC));
		
	GLfloat fAspect;

	if(cy == 0) cy = 1;

	glViewport(0, 0, cx, cy);

	fAspect = (GLfloat) cx / (GLfloat) cy;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	//gluPerspective(60.0f, fAspect, 1.0f, 10000.0f);
	//gluPerspective(45.0f, fAspect, 0.1.f, 100.0f);

	glOrtho(0, cx, 0, cy, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	VERIFY(wglMakeCurrent(NULL, NULL));
}

void CFileEntityViewer::BuildFont(void)
{
// 	HFONT font; // <1>
// 	base = glGenLists(96); // <2>
// 	font = CreateFont(-24, // <3-1>
// 				0,
// 				0,
// 				0,
// 				FW_BOLD, // <3-2>
// 				FALSE, // <3-3>
// 				FALSE, // <3-4>
// 				FALSE, // <3-5>
// 				ANSI_CHARSET, // <3-6>
// 				OUT_TT_PRECIS,
// 				CLIP_DEFAULT_PRECIS,
// 				ANTIALIASED_QUALITY,
// 				FF_DONTCARE|DEFAULT_PITCH,
// 				"Courier New"); // <3-6>
//     
// 	SelectObject(hDC, font); // <4>
// 	wglUseFontBitmaps(hDC, 32, 96, base); // <5>
}

void CFileEntityViewer::RemoveFont(void)
{
	glDeleteLists(base, 96);
}

void CFileEntityViewer::DrawText(int WinPosX, int WinPosY, CString strMsg, void* font, double Color0, double Color1, double Color2)
{
	//나중에 폰트 개선

    //double GLPosX, GLPosY, GLPosZ;
    //WinPosToWorldPos(WinPosX, WinPosY, 0, &GLPosX, &GLPosY, &GLPosZ );

    glColor3f( Color0, Color1, Color2 );
	glRasterPos3d( WinPosX , WinPosY, 0 );

    for ( int i = 0 ; i < strMsg.GetLength() ; i++ )
    {
        glutBitmapCharacter( font, strMsg.GetAt(i) );
    }
}






