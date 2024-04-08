//
//	featNamelook.h - is the declaration of the NameLook class which reads name data from an RDMS table.
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
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions (including VB)
#include <afxdb.h>          // MFC database classes

class featNameLook : public CRecordset
{
public:
//	BYTE	m_state;
//	int 	m_county;
//	long	m_feat;
	long		m_tlid;
	CString	m_prefixDir;
	CString m_prefixType;
	CString m_prefixQual;
	CString	m_name;
	CString	m_suffixDir;
	CString	m_suffixType;
	CString m_suffixQual;
	CString	m_lineArid;
	CString m_paFlag;

	BYTE stateFips;
	int		countyFips;
	long	tlid;

	featNameLook( CDatabase *pDB = 0 );

	virtual CString GetDefaultConnect();	// Default connection string
	virtual CString GetDefaultSQL(); 	// Default SQL for Recordset
	virtual void DoFieldExchange(CFieldExchange* pFX);	// RFX support
	DECLARE_DYNAMIC(featNameLook)
};
