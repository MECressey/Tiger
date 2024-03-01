//
//	namelook.cpp - is the implementation of the NameLook class which reads name data from an RDMS table.
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
#include "namelook.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(NameLook, CRecordset)

NameLook::NameLook( CDatabase* pDB )
	: CRecordset( pDB )
{
	//{{AFX_FIELD_INIT(NameLook)
//	m_state = 0;
//	m_county = 0;
//	m_feat = 0;
	m_tlid = 0;
	m_rtsq = 0;
	m_dirp = _T("");
	m_name = _T("");
	m_type = _T("");
	m_dirs = _T("");
//	m_id	 = 0;
	m_nFields = 6;
	//}}AFX_FIELD_INIT

	stateFips = 0;
	countyFips = 0;
	tlid = 0;
	this->m_nParams = 3;
	this->m_strFilter = "(T1.STATE = T2.STATE AND T1.COUNTY = T2.COUNTY AND T1.FEAT = T2.FEAT)";
	this->m_strFilter += "AND (T1.STATE = ? AND T1.COUNTY = ? AND T1.TLID = ?)";
//	this->m_strFilter += "AND (T2.ID = T3.ID)";
	this->m_strSort = "T1.STATE, T1.COUNTY, T1.TLID, T1.RTSQ";
}

CString NameLook::GetDefaultConnect()
{
	return "ODBC;DSN=TigerData;";
}

CString NameLook::GetDefaultSQL()
{
	return( _T( "[23NAMES] T1, TGRNAMES T2" ) );
}

void NameLook::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(NameLook)
	pFX->SetFieldType( CFieldExchange::outputColumn );

//  RFX_Byte(pFX, "state", m_state );
//  RFX_Int( pFX, "county", this->m_county );
//	RFX_Long(pFX, "feat", this->m_feat );
	RFX_Long(pFX, _T("T1.TLID"), this->m_tlid );
  RFX_Int( pFX, _T("T1.RTSQ"), this->m_rtsq );
  RFX_Text( pFX, _T("T2.name"), this->m_name );
  RFX_Text( pFX, _T("T2.type"), this->m_type );
  RFX_Text( pFX, _T("T2.dirp"), this->m_dirp );
  RFX_Text( pFX, _T("T2.dirs"), this->m_dirs );
//	RFX_Long(pFX, "T2.ID", this->m_id );
	//}}AFX_FIELD_MAP

  if( this->m_nParams != 0 )
  {
		pFX->SetFieldType( CFieldExchange::param );
	  
	  RFX_Byte(pFX, _T("stateFips"), this->stateFips );
	  RFX_Int( pFX, _T("countyFips"), this->countyFips );
		RFX_Long(pFX, _T("tlid"), this->tlid );
  }
}
