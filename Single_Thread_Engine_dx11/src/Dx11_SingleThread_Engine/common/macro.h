#pragma once

////////////////////////////////

#ifndef dxSAFE_RELEASE
#define dxSAFE_RELEASE(A)				{ if(A) { (A)->Release(); (A)=nullptr; } }
#endif 

#ifndef pSAFE_NEW
#define pSAFE_NEW(A, B)					{ if (!A) A = new B; }
#endif 

#ifndef pSAFE_DEL
#define pSAFE_DEL(A)					{ if (A) delete A; (A)=nullptr; }
#endif 

#ifndef pSAFE_NEW_ARRAY
#define pSAFE_NEW_ARRAY(A, B, C)		{ if (!A && C) A = new B[C]; }
#endif 

#ifndef pSAFE_DELETE_ARRAY
#define pSAFE_DELETE_ARRAY(A)			{ if (A) delete [] A; (A)=nullptr; }
#endif 

////////////////////////////////

#ifndef V
#define V(x)       { hr = (x); }
#endif
#ifndef V_RETURN
#define V_RETURN(x){ hr = (x); if( FAILED(hr) ) { return hr; } }
#endif
