#pragma once
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions (including VB)
#include <afxdb.h>
class DbBlocks : public CRecordset
{
public:
	int m_county;
	long	m_tlid;
	CString	m_blkl;
	CString	m_blkr;
	int 	m_trackl;
	int 	m_trackr;
	int	m_countyl;
	int	m_countyr;

	DbBlocks(CDatabase* pDB = 0);

	virtual CString GetDefaultConnect();	// Default connection string
	virtual CString GetDefaultSQL(); 	// Default SQL for Recordset
	virtual void DoFieldExchange(CFieldExchange* pFX);	// RFX support
	DECLARE_DYNAMIC(DbBlocks)
};

