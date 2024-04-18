//
//	DbEdgeFaces.cpp - is the implementation of the NameLook class which reads name data from an RDMS table.
//  Copyright(C) 2024 Michael E. Cressey
//
//	This program is free software : you can redistribute it and /or modify it under the terms of the
//	GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or
//	any later version.
//
//	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
//	implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License along with this program.
//  If not, see https://www.gnu.org/licenses/
//
#include "DbEdgeFaces.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(DbEdgeFaces, CRecordset)

DbEdgeFaces::DbEdgeFaces( CDatabase* pDB )
	: CRecordset( pDB )
{
	//{{AFX_FIELD_INIT(NameLook)
//	m_county = 0;
//	m_state = 0;
//	m_feat = 0;
	m_tlid = 0;
	m_nFields = 1;
	//}}AFX_FIELD_INIT

	stateFips = 0;
	countyFips = 0;
	tlid = 0;
	this->m_nParams = 2;
	this->m_strFilter = "(EF.STATEFIPS = ? AND EF.COUNTYFIPS = ?)";
	//this->m_strSort = "T1.STATEFIPS, T1.COUNTYFIPS, T1.PAFlag DESC";
}

CString DbEdgeFaces::GetDefaultConnect()
{
	return "ODBC;DSN=TigerBase;";
}

CString DbEdgeFaces::GetDefaultSQL()
{
	return(_T("MEEdgeFaces"));
}

void DbEdgeFaces::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(NameLook)
	pFX->SetFieldType( CFieldExchange::outputColumn );

//  RFX_Byte(pFX, "state", m_state );
//  RFX_Int( pFX, "county", this->m_county );
//	RFX_Long(pFX, "feat", this->m_feat );
	RFX_Long(pFX, _T("TLID"), this->m_tlid );
//	RFX_Long(pFX, "T2.ID", this->m_id );
	//}}AFX_FIELD_MAP

  if( this->m_nParams != 0 )
  {
		pFX->SetFieldType( CFieldExchange::param );
	  
	  RFX_Byte(pFX, _T("stateFips"), this->stateFips );
	  RFX_Int( pFX, _T("countyFips"), this->countyFips );
  }
}
