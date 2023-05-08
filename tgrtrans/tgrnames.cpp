#include "tgrnames.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(TgrNames, CRecordset)

TgrNames::TgrNames( CDatabase* pDB )
	: CRecordset( pDB )
{
	//{{AFX_FIELD_INIT(TgrNames)
	m_state = 0;
	m_county = 0;
	m_feat = 0;
	m_dirp = _T("");
	m_name = _T("");
	m_type = _T("");
	m_dirs = _T("");
	m_nFields = 7;
	//}}AFX_FIELD_INIT

	stateFips = 0;
	countyFips = 0;
	featNum = 0;
	qName =  _T("");
}

CString TgrNames::GetDefaultConnect()
{
	return "ODBC;DSN=TigerBase;";
}

CString TgrNames::GetDefaultSQL()
{
	return( _T( "TGRNAMES" ) );
}

void TgrNames::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(TgrNames)
	pFX->SetFieldType( CFieldExchange::outputColumn );

  RFX_Byte(pFX, _T("state"), m_state );
  RFX_Int( pFX, _T("county"), this->m_county );
	RFX_Long(pFX, _T("feat"), this->m_feat );
  RFX_Text( pFX, _T("name"), this->m_name );
  RFX_Text( pFX, _T("type"), this->m_type );
  RFX_Text( pFX, _T("dirp"), this->m_dirp );
  RFX_Text( pFX, _T("dirs"), this->m_dirs );
	//}}AFX_FIELD_MAP

  if( ! this->m_strFilter.IsEmpty() )
  {
		pFX->SetFieldType( CFieldExchange::param );
	  
	  RFX_Byte(pFX, _T("stateFips"), stateFips );
	  RFX_Int( pFX, _T("countyFips"), countyFips );

	  RFX_Text( pFX, _T("name"), this->qName );
//			RFX_Long(pFX, "featNum", featNum );
  }
}
