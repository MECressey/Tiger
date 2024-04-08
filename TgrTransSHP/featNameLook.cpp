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
#include "featNameLook.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(featNameLook, CRecordset)

featNameLook::featNameLook( CDatabase* pDB )
	: CRecordset( pDB )
{
	//{{AFX_FIELD_INIT(NameLook)
//	m_county = 0;
//	m_state = 0;
//	m_feat = 0;
	m_tlid = 0;
	m_prefixDir = _T("");
	m_prefixType = _T("");
	m_prefixQual = _T("");
	m_name = _T("");
	m_suffixDir = _T("");
	m_suffixType = _T("");
	m_suffixQual = _T("");
	m_lineArid = _T("");
	m_paFlag = _T("");
	m_nFields = 10;
	//}}AFX_FIELD_INIT

	stateFips = 0;
	countyFips = 0;
	tlid = 0;
	this->m_nParams = 3;
	this->m_strFilter = "(T1.STATEFIPS = ? AND T1.COUNTYFIPS = ? AND T1.TLID = ?)";
	this->m_strSort = "T1.STATEFIPS, T1.COUNTYFIPS, T1.PAFlag DESC";
}

CString featNameLook::GetDefaultConnect()
{
	return "ODBC;DSN=TigerData;";
}

CString featNameLook::GetDefaultSQL()
{
	return( _T( "MEFeatureNames T1" ) );
}

void featNameLook::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(NameLook)
	pFX->SetFieldType( CFieldExchange::outputColumn );

//  RFX_Byte(pFX, "state", m_state );
//  RFX_Int( pFX, "county", this->m_county );
//	RFX_Long(pFX, "feat", this->m_feat );
	RFX_Long(pFX, _T("T1.TLID"), this->m_tlid );
	RFX_Text(pFX, _T("T1.prefixDir"), this->m_prefixDir);
	RFX_Text(pFX, _T("T1.prefixType"), this->m_prefixType);
	RFX_Text(pFX, _T("T1.prefixQual"), this->m_prefixQual);
	RFX_Text(pFX, _T("T1.baseName"), this->m_name);
	RFX_Text(pFX, _T("T1.suffixDir"), this->m_suffixDir);
	RFX_Text( pFX, _T("T1.suffixType"), this->m_suffixType);
	RFX_Text(pFX, _T("T1.suffixQual"), this->m_suffixQual);
  RFX_Text( pFX, _T("T1.lineArid"), this->m_lineArid);
  RFX_Text( pFX, _T("T1.PAFlag"), this->m_paFlag);
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
