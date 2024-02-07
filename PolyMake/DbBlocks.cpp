#include "DbBlocks.h"


IMPLEMENT_DYNAMIC(DbBlocks, CRecordset)

DbBlocks::DbBlocks(CDatabase* pDB)
	: CRecordset(pDB)
{
	//{{AFX_FIELD_INIT(DbBlocks)
	m_county = 0;
	m_tlid = 0;
	m_trackl = 0;
	m_trackr = 0;
	m_countyl = 0;
	m_countyr = 0;
	m_blkl = _T("");
	m_blkr = _T("");
	m_nFields = 3;
	//}}AFX_FIELD_INIT

}

CString DbBlocks::GetDefaultConnect()
{
	return "ODBC;DSN=TigerBase;";
}

CString DbBlocks::GetDefaultSQL()
{
	return(_T("MEBlocks"));
}

void DbBlocks::DoFieldExchange(CFieldExchange* pFX)
{
	//{{AFX_FIELD_MAP(TgrNames)
	pFX->SetFieldType(CFieldExchange::outputColumn);

	/*RFX_Int(pFX, _T("trackl"), this->m_trackl);
	RFX_Int(pFX, _T("trackr"), this->m_trackr);
	RFX_Int(pFX, _T("countyl"), this->m_countyl);
	RFX_Int(pFX, _T("countrr"), this->m_countyr);*/
	RFX_Long(pFX, _T("tlid"), this->m_tlid);
	RFX_Text(pFX, _T("blkl"), this->m_blkl);
	RFX_Text(pFX, _T("blkr"), this->m_blkr);
	//}}AFX_FIELD_MAP
/*
	if (!this->m_strFilter.IsEmpty())
	{
		pFX->SetFieldType(CFieldExchange::param);

		RFX_Byte(pFX, _T("stateFips"), stateFips);
		RFX_Int(pFX, _T("countyFips"), countyFips);

		RFX_Text(pFX, _T("name"), this->qName);
		//			RFX_Long(pFX, "featNum", featNum );
	}
	*/
}

