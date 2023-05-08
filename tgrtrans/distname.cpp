#include "distname.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(DistNames, CRecordset)

DistNames::DistNames( CDatabase* pDB )
	: CRecordset( pDB )
{
	//{{AFX_FIELD_INIT(DistNames)
	m_name = _T("");
	m_id	 = 0;
	m_nFields = 2;
	//}}AFX_FIELD_INIT

	this->nameParam = _T("");
	this->idParam = 0;
	this->m_strFilter = "(NAME = ?)";
	this->m_nParams = 1;
}

CString DistNames::GetDefaultConnect()
{
	return "ODBC;DSN=TigerData;";
}

CString DistNames::GetDefaultSQL()
{
	return( _T( "DISTNAMES" ) );
}

void DistNames::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(DistNames)
	pFX->SetFieldType( CFieldExchange::outputColumn );

  RFX_Text( pFX, "name", this->m_name );
	RFX_Long(pFX, "ID", this->m_id );
	//}}AFX_FIELD_MAP

  if( ! this->m_strFilter.IsEmpty() )
  {
		pFX->SetFieldType( CFieldExchange::param );
	  
	  RFX_Text( pFX, "name", this->nameParam );
//		RFX_Long( pFX, "id", this->idParam );
  }
}
