//
//	dogtpoly.h - is the declaration of methods that writes Tiger GT Polygon/Landmark data to several text files.
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
void DoGTpoly( FILE *, const TigerRecI & );

void DoLMlink( FILE *, const TigerRec8 & );

void DoLMarea( FILE *, const TigerRec7 & );

void DoLMpoint( FILE *, const TigerRec7 & );
