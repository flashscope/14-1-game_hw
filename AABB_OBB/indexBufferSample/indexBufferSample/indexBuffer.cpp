#include <d3d9.h>
#include <d3dx9.h>
#include <stdio.h>
#include <string>
#pragma warning (disable : 4996)

/**-----------------------------------------------------------------------------
* \brief 인덱스버퍼 생성
* 파일: IndexBuffer.cpp
*
* 설명: 인덱스 버퍼(Index Buffer)란 정점을 보관하기 위한 정점버퍼(VB)처럼
*       인덱스를 보관하기위한 전용 객체이다. D3D 학습예제에는 이러한 예제가
*       IB를 사용한 예제가 없기 때문에 새롭게 추가한 것이다.
*------------------------------------------------------------------------------
*/

/**-----------------------------------------------------------------------------
*  전역변수
*------------------------------------------------------------------------------
*/
LPDIRECT3D9             g_pD3D = NULL; /// D3D 디바이스를 생성할 D3D객체변수
LPDIRECT3DDEVICE9       g_pd3dDevice = NULL; /// 렌더링에 사용될 D3D디바이스
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL; /// 정점을 보관할 정점버퍼
LPDIRECT3DINDEXBUFFER9	g_pIB = NULL; /// 인덱스를 보관할 인덱스버퍼
LPDIRECT3DVERTEXBUFFER9 g_pVB2 = NULL; /// 정점을 보관할 정점버퍼
LPDIRECT3DINDEXBUFFER9	g_pIB2 = NULL; /// 인덱스를 보관할 인덱스버퍼

UINT countHit = 0;
FLOAT controlX = 0.0f;
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


struct BoundingBox
{
	// for AABB
	D3DXVECTOR3 minMin, maxMax;

	// for OBB
	D3DXVECTOR3 center;
	D3DXVECTOR3 vAxisDir[3]; //상자에 평행한 세 축의 단위벡터
	float  fAxisLen[3];
};

BoundingBox g_BoxA;
BoundingBox g_BoxB;
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////




/// 사용자 정점 구조체에 관한 정보를 나타내는 FVF값
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ)

struct MYINDEX
{
	WORD	_0, _1, _2;		/// 일반적으로 인덱스는 16비트의 크기를 갖는다.
	/// 32비트의 크기도 가능하지만 구형 그래픽카드에서는 지원되지 않는다.
};


/**-----------------------------------------------------------------------------
* Direct3D 초기화
*------------------------------------------------------------------------------
*/
HRESULT InitD3D( HWND hWnd )
{

	/// 디바이스를 생성하기위한 D3D객체 생성
	if ( NULL == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
		return E_FAIL;

	/// 디바이스를 생성할 구조체
	/// 복잡한 오브젝트를 그릴것이기때문에, 이번에는 Z버퍼가 필요하다.
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory( &d3dpp, sizeof( d3dpp ) );
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	/// 디바이스 생성
	if ( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp, &g_pd3dDevice ) ) )
	{
		return E_FAIL;
	}


	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, TRUE );

	/// Z버퍼기능을 켠다.
	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

	/// 정점에 색깔값이 있으므로, 광원기능을 끈다.
	g_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


float RandomFloat( float min, float max )
{
	float random = ( (float)rand( ) ) / (float)RAND_MAX;
	float diff = max - min;
	float range = random * diff;
	
	return (min + range);
}

void GetRandomVertex( D3DVECTOR* vector, float min, float max, int size )
{
	for ( int i = 0; i < size; ++i )
	{
		vector[i].x = RandomFloat( min, max );
		vector[i].y = RandomFloat( min, max );
		vector[i].z = RandomFloat( min, max );
	}
}


void CalcBoundingBox()
{
	D3DVECTOR vectorArray[2000];
	int size = sizeof( vectorArray ) / sizeof( D3DVECTOR );
	GetRandomVertex( vectorArray, -1, 1, size );

	//////////////////////////////////////////////////////////////////////////
	vectorArray[0].x = 3.5f;
	//////////////////////////////////////////////////////////////////////////
	if ( size < 0)
	{
		return;
	}

	FLOAT minX = vectorArray[0].x;
	FLOAT minY = vectorArray[0].y;
	FLOAT minZ = vectorArray[0].z;
	FLOAT maxX = vectorArray[0].x;
	FLOAT maxY = vectorArray[0].y;
	FLOAT maxZ = vectorArray[0].z;

	for ( int i = 1; i < size; ++i )
	{
		maxX = __max( vectorArray[i].x, maxX );
		maxY = __max( vectorArray[i].y, maxY );
		maxZ = __max( vectorArray[i].z, maxZ );

		minX = __min( vectorArray[i].x, minX );
		minY = __min( vectorArray[i].y, minY );
		minZ = __min( vectorArray[i].z, minZ );
		
	}


	g_BoxA.minMin = { minX, minY, minZ };
	g_BoxA.maxMax = { maxX, maxY, maxZ };



	g_BoxA.center = { minX + ( maxX - minX ) / 2, minY + ( maxY - minY ) / 2, minZ+( maxZ - minZ ) / 2 };
	
	g_BoxA.fAxisLen[0] = ( maxX - minX ) / 2; // x
	g_BoxA.fAxisLen[1] = ( maxY - minY ) / 2; // y
	g_BoxA.fAxisLen[2] = ( maxZ - minZ ) / 2; // z

	g_BoxA.vAxisDir[0] = { 1.0f, 0.f, 0.f };
	g_BoxA.vAxisDir[1] = { 0.0f, 1.f, 0.f };
	g_BoxA.vAxisDir[2] = { 0.0f, 0.f, 1.f };

}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


/**-----------------------------------------------------------------------------
* 정점버퍼를 생성하고 정점값을 채워넣는다.
* 정점버퍼란 기본적으로 정점정보를 갖고있는 메모리블럭이다.
* 정점버퍼를 생성한 다음에는 반드시 Lock()과 Unlock()으로 포인터를 얻어내서
* 정점정보를 정점버퍼에 써넣어야 한다.
* 또한 D3D는 인덱스버퍼도 사용가능하다는 것을 명심하자.
* 정점버퍼나 인덱스버퍼는 기본 시스템 메모리외에 디바이스 메모리(비디오카드 메모리)
* 에 생성될수 있는데, 대부분의 비디오카드에서는 이렇게 할경우 엄청난 속도의 향상을
* 얻을 수 있다.
*------------------------------------------------------------------------------
*/



// un safe
void GetVerticesBoxs( BoundingBox* box, D3DXVECTOR3* vertices )
{
	vertices[0] = { box->minMin.x, box->maxMax.y, box->maxMax.z };
	vertices[1] = { box->maxMax.x, box->maxMax.y, box->maxMax.z };
	vertices[2] = { box->maxMax.x, box->maxMax.y, box->minMin.z };
	vertices[3] = { box->minMin.x, box->maxMax.y, box->minMin.z };

	vertices[4] = { box->minMin.x, box->minMin.y, box->maxMax.z };
	vertices[5] = { box->maxMax.x, box->minMin.y, box->maxMax.z };
	vertices[6] = { box->maxMax.x, box->minMin.y, box->minMin.z };
	vertices[7] = { box->minMin.x, box->minMin.y, box->minMin.z };
}

HRESULT InitVB()
{
	CalcBoundingBox();
	
	//boundingBox.minMin.
	/// 상자(cube)를 렌더링하기위해 8개의 정점을 선언
	D3DXVECTOR3 vertices[8];
	GetVerticesBoxs( &g_BoxA, vertices );


	FLOAT minX = -3.0f;
	FLOAT minY = -1.0f;
	FLOAT minZ = 0.0f;
	FLOAT maxX = -2.0f;
	FLOAT maxY = 0.0f;
	FLOAT maxZ = 1.0f;
	g_BoxB.minMin = { minX, minY, minZ };
	g_BoxB.maxMax = { maxX, maxY, maxZ };

	g_BoxB.center = { minX + ( maxX - minX ) / 2, minY + ( maxY - minY ) / 2, minZ + ( maxZ - minZ ) / 2 };
	//g_BoxB.center = { ( maxX - minX ) / 2, ( maxY - minY ) / 2, ( maxZ - minZ ) / 2 };

	g_BoxB.fAxisLen[0] = ( maxX - minX ) / 2; // x
	g_BoxB.fAxisLen[1] = ( maxY - minY ) / 2; // y
	g_BoxB.fAxisLen[2] = ( maxZ - minZ ) / 2; // z

	g_BoxB.vAxisDir[0] = { 1.0f, 0.f, 0.f };
	g_BoxB.vAxisDir[1] = { 0.0f, 1.f, 0.f };
	g_BoxB.vAxisDir[2] = { 0.0f, 0.f, 1.f };



	D3DXVECTOR3 vertices2[8];
	GetVerticesBoxs( &g_BoxB, vertices2 );

	//obb->c = ( min + max ) * 0.5f + g_worldPos;

	/// 정점버퍼 생성
	/// 8개의 사용자정점을 보관할 메모리를 할당한다.
	/// FVF를 지정하여 보관할 데이터의 형식을 지정한다.
	if ( FAILED( g_pd3dDevice->CreateVertexBuffer( 8 * sizeof( D3DXVECTOR3 ),
		0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &g_pVB, NULL ) ) )
	{
		return E_FAIL;
	}

	if ( FAILED( g_pd3dDevice->CreateVertexBuffer( 8 * sizeof( D3DXVECTOR3 ),
		0, D3DFVF_CUSTOMVERTEX,
		D3DPOOL_DEFAULT, &g_pVB2, NULL ) ) )
	{
		return E_FAIL;
	}
	/// 정점버퍼를 값으로 채운다. 
	/// 정점버퍼의 Lock()함수를 호출하여 포인터를 얻어온다.
	VOID* pVertices;
	if ( FAILED( g_pVB->Lock( 0, sizeof( vertices ), (void**) &pVertices, 0 ) ) )
		return E_FAIL;
	memcpy( pVertices, vertices, sizeof( vertices ) );
	g_pVB->Unlock();


	VOID* pVertices2;
	if ( FAILED( g_pVB2->Lock( 0, sizeof( vertices2 ), (void**)&pVertices2, 0 ) ) )
		return E_FAIL;
	memcpy( pVertices2, vertices2, sizeof( vertices2 ) );
	g_pVB2->Unlock();

	return S_OK;
}


HRESULT InitIB()
{
	/// 상자(cube)를 렌더링하기위해 12개의 면을 선언
	MYINDEX	indices[] =
	{
		{ 0, 1, 2 }, { 0, 2, 3 },	/// 윗면
		{ 4, 6, 5 }, { 4, 7, 6 },	/// 아랫면
		{ 0, 3, 7 }, { 0, 7, 4 },	/// 왼면
		{ 1, 5, 6 }, { 1, 6, 2 },	/// 오른면
		{ 3, 2, 6 }, { 3, 6, 7 },	/// 앞면
		{ 0, 4, 5 }, { 0, 5, 1 }	/// 뒷면
	};

	/// 인덱스버퍼 생성
	/// D3DFMT_INDEX16은 인덱스의 단위가 16비트 라는 것이다.
	/// 우리는 MYINDEX 구조체에서 WORD형으로 선언했으므로 D3DFMT_INDEX16을 사용한다.
	if ( FAILED( g_pd3dDevice->CreateIndexBuffer( 12 * sizeof( MYINDEX ), 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &g_pIB, NULL ) ) )
	{
		return E_FAIL;
	}

	if ( FAILED( g_pd3dDevice->CreateIndexBuffer( 12 * sizeof( MYINDEX ), 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &g_pIB2, NULL ) ) )
	{
		return E_FAIL;
	}

	/// 인덱스버퍼를 값으로 채운다. 
	/// 인덱스버퍼의 Lock()함수를 호출하여 포인터를 얻어온다.
	VOID* pIndices;
	if ( FAILED( g_pIB->Lock( 0, sizeof( indices ), (void**) &pIndices, 0 ) ) )
		return E_FAIL;
	memcpy( pIndices, indices, sizeof( indices ) );
	g_pIB->Unlock();

	VOID* pIndices2;
	if ( FAILED( g_pIB2->Lock( 0, sizeof( indices ), (void**)&pIndices2, 0 ) ) )
		return E_FAIL;
	memcpy( pIndices2, indices, sizeof( indices ) );
	g_pIB2->Unlock();
	return S_OK;
}









/**-----------------------------------------------------------------------------
* 행렬 설정
*------------------------------------------------------------------------------
*/
VOID SetupMatrices()
{

	/// 월드행렬
	D3DXMATRIXA16 matWorld;
	D3DXMatrixIdentity( &matWorld );							/// 월드행렬을 단위행렬으로 설정
	D3DXMatrixRotationY( &matWorld, GetTickCount() / 500.0f );	/// Y축을 중심으로 회전행렬 생성
	g_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );		/// 디바이스에 월드행렬 설정

	/// 뷰행렬을 설정
	D3DXVECTOR3 vEyePt( 0.0f, 3.0f, -5.0f );
	D3DXVECTOR3 vLookatPt( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 vUpVec( 0.0f, 1.0f, 0.0f );
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
	g_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

	/// 프로젝션 행렬 설정
	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI / 2, 1.0f, 1.0f, 100.0f );
	g_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );
}



/**-----------------------------------------------------------------------------
* 초기화 객체들 소거
*------------------------------------------------------------------------------
*/
VOID Cleanup()
{
	if ( g_pIB != NULL )
		g_pIB->Release();

	if ( g_pVB != NULL )
		g_pVB->Release();

	if ( g_pd3dDevice != NULL )
		g_pd3dDevice->Release();

	if ( g_pD3D != NULL )
		g_pD3D->Release();
}




/**-----------------------------------------------------------------------------
* 화면 그리기
*------------------------------------------------------------------------------
*/
VOID Render()
{
	/// 후면버퍼와 Z버퍼 초기화
	g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0, 0, 255 ), 1.0f, 0 );

	// 행렬설정
	SetupMatrices();

	/// 렌더링 시작
	if ( SUCCEEDED( g_pd3dDevice->BeginScene() ) )
	{
		/// 정점버퍼의 삼각형을 그린다.
		/// 1. 정점정보가 담겨있는 정점버퍼를 출력 스트림으로 할당한다.
		g_pd3dDevice->SetStreamSource( 0, g_pVB, 0, sizeof( D3DXVECTOR3 ) );
		/// 2. D3D에게 정점쉐이더 정보를 지정한다. 대부분의 경우에는 FVF만 지정한다.
		g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
		/// 3. 인덱스버퍼를 지정한다.
		g_pd3dDevice->SetIndices( g_pIB );

		g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
		/// 4. DrawIndexedPrimitive()를 호출한다.
		g_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12 );




		//////////////////////////////////////////////////////////////////////////

		g_pd3dDevice->SetStreamSource( 0, g_pVB2, 0, sizeof( D3DXVECTOR3 ) );
		g_pd3dDevice->SetFVF( D3DFVF_CUSTOMVERTEX );
		g_pd3dDevice->SetIndices( g_pIB2 );
		g_pd3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
		g_pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12 );
		//////////////////////////////////////////////////////////////////////////

		/// 렌더링 종료
		g_pd3dDevice->EndScene();
	}

	/// 후면버퍼를 보이는 화면으로!
	g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}




/**-----------------------------------------------------------------------------
* 윈도우 프로시져
*------------------------------------------------------------------------------
*/
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg )
	{
		case WM_DESTROY:
			Cleanup();
			PostQuitMessage( 0 );
			return 0;
	}

	return DefWindowProc( hWnd, msg, wParam, lParam );
}

bool CheckAABB()
{
	if ( g_BoxA.minMin.x <= g_BoxB.maxMax.x && g_BoxA.maxMax.x >= g_BoxB.minMin.x &&
		 g_BoxA.minMin.y <= g_BoxB.maxMax.y && g_BoxA.maxMax.y >= g_BoxB.minMin.y &&
		 g_BoxA.minMin.z <= g_BoxB.maxMax.z && g_BoxA.maxMax.z >= g_BoxB.minMin.z )
	{
		return TRUE;
	}
	return FALSE;

}

BOOL CheckOBB(  )
{

	double c[3][3];
	double absC[3][3];
	double d[3];
	double r0, r1, r;
	int i;
	const double cutoff = 0.999999;
	bool existsParallelPair = false;
	D3DXVECTOR3 diff = g_BoxA.center - g_BoxB.center;

	for ( i = 0; i < 3; ++i )
	{
		c[0][i] = D3DXVec3Dot( &g_BoxA.vAxisDir[0], &g_BoxB.vAxisDir[i] );
		absC[0][i] = abs( c[0][i] );
		if ( absC[0][i] > cutoff )
			existsParallelPair = true;
	}
	d[0] = D3DXVec3Dot( &diff, &g_BoxA.vAxisDir[0] );
	r = abs( d[0] );
	r0 = g_BoxA.fAxisLen[0];
	r1 = g_BoxB.fAxisLen[0] * absC[0][0] + g_BoxB.fAxisLen[1] * absC[0][1] + g_BoxB.fAxisLen[2] * absC[0][2];
	if ( r > r0 + r1 )
		return FALSE;

	for ( i = 0; i < 3; ++i )
	{
		c[1][i] = D3DXVec3Dot( &g_BoxA.vAxisDir[1], &g_BoxB.vAxisDir[i] );
		absC[1][i] = abs( c[1][i] );
		if ( absC[1][i] > cutoff )
			existsParallelPair = true;
	}
	d[1] = D3DXVec3Dot( &diff, &g_BoxA.vAxisDir[1] );
	r = abs( d[1] );
	r0 = g_BoxA.fAxisLen[1];
	r1 = g_BoxB.fAxisLen[0] * absC[1][0] + g_BoxB.fAxisLen[1] * absC[1][1] + g_BoxB.fAxisLen[2] * absC[1][2];
	if ( r > r0 + r1 )
		return FALSE;

	for ( i = 0; i < 3; ++i )
	{
		c[2][i] = D3DXVec3Dot( &g_BoxA.vAxisDir[2], &g_BoxB.vAxisDir[i] );
		absC[2][i] = abs( c[2][i] );
		if ( absC[2][i] > cutoff )
			existsParallelPair = true;
	}
	d[2] = D3DXVec3Dot( &diff, &g_BoxA.vAxisDir[2] );
	r = abs( d[2] );
	r0 = g_BoxA.fAxisLen[2];
	r1 = g_BoxB.fAxisLen[0] * absC[2][0] + g_BoxB.fAxisLen[1] * absC[2][1] + g_BoxB.fAxisLen[2] * absC[2][2];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( D3DXVec3Dot( &diff, &g_BoxB.vAxisDir[0] ) );
	r0 = g_BoxA.fAxisLen[0] * absC[0][0] + g_BoxA.fAxisLen[1] * absC[1][0] + g_BoxA.fAxisLen[2] * absC[2][0];
	r1 = g_BoxB.fAxisLen[0];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( D3DXVec3Dot( &diff, &g_BoxB.vAxisDir[1] ) );
	r0 = g_BoxA.fAxisLen[0] * absC[0][1] + g_BoxA.fAxisLen[1] * absC[1][1] + g_BoxA.fAxisLen[2] * absC[2][1];
	r1 = g_BoxB.fAxisLen[1];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( D3DXVec3Dot( &diff, &g_BoxB.vAxisDir[2] ) );
	r0 = g_BoxA.fAxisLen[0] * absC[0][2] + g_BoxA.fAxisLen[1] * absC[1][2] + g_BoxA.fAxisLen[2] * absC[2][2];
	r1 = g_BoxB.fAxisLen[2];
	if ( r > r0 + r1 )
		return FALSE;

	if ( existsParallelPair == true )
		return TRUE;

	r = abs( d[2] * c[1][0] - d[1] * c[2][0] );
	r0 = g_BoxA.fAxisLen[1] * absC[2][0] + g_BoxA.fAxisLen[2] * absC[1][0];
	r1 = g_BoxB.fAxisLen[1] * absC[0][2] + g_BoxB.fAxisLen[2] * absC[0][1];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( d[2] * c[1][1] - d[1] * c[2][1] );
	r0 = g_BoxA.fAxisLen[1] * absC[2][1] + g_BoxA.fAxisLen[2] * absC[1][1];
	r1 = g_BoxB.fAxisLen[0] * absC[0][2] + g_BoxB.fAxisLen[2] * absC[0][0];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( d[2] * c[1][2] - d[1] * c[2][2] );
	r0 = g_BoxA.fAxisLen[1] * absC[2][2] + g_BoxA.fAxisLen[2] * absC[1][2];
	r1 = g_BoxB.fAxisLen[0] * absC[0][1] + g_BoxB.fAxisLen[1] * absC[0][0];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( d[0] * c[2][0] - d[2] * c[0][0] );
	r0 = g_BoxA.fAxisLen[0] * absC[2][0] + g_BoxA.fAxisLen[2] * absC[0][0];
	r1 = g_BoxB.fAxisLen[1] * absC[1][2] + g_BoxB.fAxisLen[2] * absC[1][1];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( d[0] * c[2][1] - d[2] * c[0][1] );
	r0 = g_BoxA.fAxisLen[0] * absC[2][1] + g_BoxA.fAxisLen[2] * absC[0][1];
	r1 = g_BoxB.fAxisLen[0] * absC[1][2] + g_BoxB.fAxisLen[2] * absC[1][0];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( d[0] * c[2][2] - d[2] * c[0][2] );
	r0 = g_BoxA.fAxisLen[0] * absC[2][2] + g_BoxA.fAxisLen[2] * absC[0][2];
	r1 = g_BoxB.fAxisLen[0] * absC[1][1] + g_BoxB.fAxisLen[1] * absC[1][0];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( d[1] * c[0][0] - d[0] * c[1][0] );
	r0 = g_BoxA.fAxisLen[0] * absC[1][0] + g_BoxA.fAxisLen[1] * absC[0][0];
	r1 = g_BoxB.fAxisLen[1] * absC[2][2] + g_BoxB.fAxisLen[2] * absC[2][1];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( d[1] * c[0][1] - d[0] * c[1][1] );
	r0 = g_BoxA.fAxisLen[0] * absC[1][1] + g_BoxA.fAxisLen[1] * absC[0][1];
	r1 = g_BoxB.fAxisLen[0] * absC[2][2] + g_BoxB.fAxisLen[2] * absC[2][0];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( d[1] * c[0][2] - d[0] * c[1][2] );
	r0 = g_BoxA.fAxisLen[0] * absC[1][2] + g_BoxA.fAxisLen[1] * absC[0][2];
	r1 = g_BoxB.fAxisLen[0] * absC[2][1] + g_BoxB.fAxisLen[1] * absC[2][0];
	if ( r > r0 + r1 )
		return FALSE;

	return TRUE;
}

VOID MoveBox()
{

	g_BoxB.maxMax.x = g_BoxB.maxMax.x + controlX;
	g_BoxB.minMin.x = g_BoxB.minMin.x + controlX;

	
	//g_BoxB.center = { minX + ( maxX - minX ) / 2, minY + ( maxY - minY ) / 2, minZ + ( maxZ - minZ ) / 2 };
	g_BoxB.center.x = g_BoxB.center.x + controlX;
	D3DXVECTOR3 vertices[8];
	GetVerticesBoxs( &g_BoxB, vertices );
	//printf_s( "[%f] \n", vertices[0].x );


	if ( CheckAABB( ) )
	{
		++countHit;
		printf_s( "[%d]AABBING! \n", countHit );
	}

	if ( CheckOBB() )
	{
		++countHit;
		printf_s( "[%d]OOBB! \n", countHit );
	}


	VOID* pVertices;
	if ( FAILED( g_pVB2->Lock( 0, sizeof( vertices ), (void**)&pVertices, 0 ) ) )
	{
		g_pd3dDevice->EndScene();
		g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
		return;
	}
	memcpy( pVertices, vertices, sizeof( vertices ) );
	g_pVB2->Unlock();

	

}

/**-----------------------------------------------------------------------------
* 프로그램 시작점
*------------------------------------------------------------------------------
*/
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{

	AllocConsole();
	freopen( "CONOUT$", "wt", stdout );
	//freopen_s( "CONOUT$", "w", stdout );

	/// 윈도우 클래스 등록
	WNDCLASSEX wc = { sizeof( WNDCLASSEX ), CS_CLASSDC, MsgProc, 0L, 0L,
		GetModuleHandle( NULL ), NULL, NULL, NULL, NULL,
		L"D3D Tutorial", NULL };
	RegisterClassEx( &wc );

	/// 윈도우 생성
	HWND hWnd = CreateWindow( L"D3D Tutorial", L"D3D Tutorial 07: IndexBuffer",
							  WS_OVERLAPPEDWINDOW, 100, 100, 300, 300,
							  GetDesktopWindow(), NULL, wc.hInstance, NULL );

	/// Direct3D 초기화
	if ( SUCCEEDED( InitD3D( hWnd ) ) )
	{
		/// 정점버퍼 초기화
		if ( SUCCEEDED( InitVB() ) )
		{
			/// 인덱스버퍼 초기화
			if ( SUCCEEDED( InitIB() ) )
			{
				/// 윈도우 출력
				ShowWindow( hWnd, SW_SHOWDEFAULT );
				UpdateWindow( hWnd );

				/// 메시지 루프
				MSG msg;
				ZeroMemory( &msg, sizeof( msg ) );
				while ( msg.message != WM_QUIT )
				{
					/// 메시지큐에 메시지가 있으면 메시지 처리
					if ( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
					{
						TranslateMessage( &msg );
						DispatchMessage( &msg );
					}
					else
					{
						/// 처리할 메시지가 없으면 Render()함수 호출
						Render();
					}
					if ( msg.message == WM_KEYDOWN )
					{
						switch ( static_cast<unsigned char>( msg.wParam ) )
						{
							case 37:
								controlX = -0.1f;
								MoveBox();
								//printf_s( "left \n" );
								break;
							case 39:
								controlX = 0.1f;
								MoveBox( );
								//printf_s( "right \n" );
								break;
						}
					}

				}
			}
		}
	}

	/// 등록된 클래스 소거
	UnregisterClass( L"D3D Tutorial", wc.hInstance );
	FreeConsole( );
	return 0;
}