#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions (including VB)
#include <afxdb.h>          // MFC database classes

class NameLook : public CRecordset
{
public:
//	BYTE	m_state;
//	int 	m_county;
//	long	m_feat;
	long		m_tlid;
	int			m_rtsq;
	CString	m_dirp;
	CString	m_name;
	CString	m_type;
	CString	m_dirs;
//	long		m_id;

	BYTE stateFips;
	int		countyFips;
	long	tlid;

	NameLook( CDatabase *pDB = 0 );

	virtual CString GetDefaultConnect();	// Default connection string
	virtual CString GetDefaultSQL(); 	// Default SQL for Recordset
	virtual void DoFieldExchange(CFieldExchange* pFX);	// RFX support
	DECLARE_DYNAMIC(NameLook)
};
