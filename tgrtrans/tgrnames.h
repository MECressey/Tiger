#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions (including VB)
#include <afxdb.h>          // MFC database classes

class TgrNames : public CRecordset
{
public:
	BYTE	m_state;
	int 	m_county;
	long	m_feat;
	CString	m_dirp;
	CString	m_name;
	CString	m_type;
	CString	m_dirs;

	BYTE stateFips;
	int		countyFips;
	long	featNum;
	CString qName;

	TgrNames( CDatabase *pDB = 0 );

	virtual CString GetDefaultConnect();	// Default connection string
	virtual CString GetDefaultSQL(); 	// Default SQL for Recordset
	virtual void DoFieldExchange(CFieldExchange* pFX);	// RFX support
	DECLARE_DYNAMIC(TgrNames)
};
