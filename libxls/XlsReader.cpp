/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * Copyright 2004 Komarov Valery
 * Copyright 2006 Christophe Leitienne
 * Copyright 2008-2017 David Hoerl
 * Copyright 2013 Bob Colbert
 * Copyright 2013-2018 Evan Miller
 *
 * This file is part of libxls -- A multiplatform, C/C++ library for parsing
 * Excel(TM) files.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ''AS
 * IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <cstring>
#include <exception>
#include <string>
#include <cassert>

#include "XlsReader.h"


using namespace std;

#if XLS_WIDE_STRINGS == 1
#define XLS_STR_SPEC "%ls"
#else
#define XLS_STR_SPEC "%s"
#endif

//#define THROW_STR "XlsReader: "

namespace xls
{

static const char* error = "Error";

#if XLS_WIDE_STRINGS == 1
static const char *outConv = 
#ifndef __APPLE__
	"wchar_t";
#else
	"UCS-4-INTERNAL";
#endif
#endif

#if XLS_WIDE_STRINGS == 0
WorkBook::WorkBook(const string& fileName, int debug, const char *characterSet) :
	charSet(characterSet),
	isUTF8(!strcmp(characterSet, "UTF-8")),
#else
WorkBook::WorkBook(const string& fileName, int debug) :
	charSet("UTF-8"),
	isUTF8(true),
	iconvCD(iconv_open(outConv, "UTF-8")),
#endif
	numSheets(0),
	workBook(NULL),
	activeWorkSheetID(-1),
	activeWorkSheet(NULL),
	summary(NULL)
{
    xls_error_t error = LIBXLS_OK;
	xls(debug);

#if XLS_WIDE_STRINGS == 1
	assert(iconvCD != (iconv_t)(-1));
#endif
	workBook = xls_open_file(fileName.c_str(), charSet, &error);
	if(workBook) {
		numSheets = workBook->sheets.count;
		xls_parseWorkBook(workBook);
		summary = xls_summaryInfo(workBook);
	} else {
		throw XlsException(xls_getError(error));
	}
}
WorkBook::~WorkBook()
{
#if XLS_WIDE_STRINGS == 1
	iconv_close(iconvCD);
#endif
	xls_close_summaryInfo(summary);
	xls_close_WS(activeWorkSheet);
	xls_close_WB(workBook);	// handles nil parameter
}

uint32_t WorkBook::GetSheetCount() const
{
	return numSheets;
}

string WorkBook::GetLibraryVersion() const
{
	return string(xls_getVersion());
}

xlsString WorkBook::GetSheetName(uint32_t sheetNum) const
{
	return char2string(sheetNum < numSheets ? workBook->sheets.sheet[sheetNum].name : error);
}

bool WorkBook::GetSheetVisible(uint32_t sheetNum) const
{
	return sheetNum < numSheets ? workBook->sheets.sheet[sheetNum].visibility :  false;
}


void WorkBook::OpenSheet(uint32_t sheetNum)
{	
	if(sheetNum >= numSheets) {
		throw XlsException("no such sheet exists!");
	} else
	if(sheetNum != activeWorkSheetID) {
		activeWorkSheetID = sheetNum;
		xls_close_WS(activeWorkSheet);
		activeWorkSheet = xls_getWorkSheet(workBook, sheetNum);
		xls_parseWorkSheet(activeWorkSheet);
	}
}

void WorkBook::InitIterator(uint32_t sheetNum)
{
	if(sheetNum != UINT32_MAX) {
		OpenSheet(sheetNum);
		iterating = true;
		lastColIndex = 0;
		lastRowIndex = 0;
	} else {
		iterating = false;
	}
}

cellContent WorkBook::GetNextCell(void)
{
	cellContent content;

	if(!iterating) throw XlsException("asked for the next cell, but not iterating!");
	
	uint32_t numRows = activeWorkSheet->rows.lastrow + 1;
	uint32_t numCols = activeWorkSheet->rows.lastcol + 1;

	if(lastRowIndex >= numRows) return content;
	
	for (uint32_t t=lastRowIndex; t<numRows; t++)
	{
		xlsRow *rowP = &activeWorkSheet->rows.row[t];
		for (uint32_t tt=lastColIndex; tt<numCols; tt++)
		{
			xlsCell	*cell = &rowP->cells.cell[tt];
			
			if(cell->id == XLS_RECORD_BLANK) continue;
			lastColIndex = tt + 1;
			FormatCell(cell, content);
			return content;
		}
		++lastRowIndex;
		lastColIndex = 0;
	}
	// don't make iterator false - user can keep asking for cells, they all just be blank ones though
	return content;
}

uint32_t WorkBook::GetNumRows()
{
    if(activeWorkSheet == NULL)
        return -1;
    return activeWorkSheet->rows.lastrow + 1;
}

uint32_t WorkBook::GetNumCols()
{
    if(activeWorkSheet == NULL)
        return -1;
    return activeWorkSheet->rows.lastcol + 1;
}

cellContent WorkBook::GetCell(uint32_t workSheetIndex, uint16_t row, uint16_t col)
{
	cellContent content;
	
	OpenSheet(workSheetIndex);
	
	uint32_t numRows = activeWorkSheet->rows.lastrow + 1;
	uint32_t numCols = activeWorkSheet->rows.lastcol + 1;

	for (uint32_t t=0; t<numRows; t++)
	{
		xlsRow *rowP = &activeWorkSheet->rows.row[t];
		for (uint32_t tt=0; tt<numCols; tt++)
		{
			xlsCell	*cell = &rowP->cells.cell[tt];
			if(cell->row < row) break;
			if(cell->row > row) return content;
			
			if(cell->id == XLS_RECORD_BLANK) continue;
			
			if(cell->col == col) {
				FormatCell(cell, content);
				return content;
			}
		}
	}
	
	return content;
}

cellContent WorkBook::GetCell(uint32_t workSheetIndex, uint16_t row, const char *colStr)
{
	int32_t col;
	
	if(strlen(colStr) > 2 || strlen(colStr) == 0) throw XlsException("incorrect column specifier");

	col = colStr[0] - 'A';
	if(col < 0 || col >= 26) throw XlsException("incorrect column specifier");
	char c = colStr[1];
	if(c) {
		col *= 26;
		int32_t col2 = c - 'A';
		if(col2 < 0 || col2 >= 26) throw XlsException("incorrect column specifier");
		col += col2;
	}

	return GetCell(workSheetIndex, row, col);
}

bool WorkBook::isDateTime(xlsCell *cell) const
{
    // 1. Получаем расширенное форматирование ячейки (Extended Format)
    // cell->xf — это индекс в таблице xfs
    // Если индекс за пределами размера таблицы xfs, возвращаем не DateTime
    if (cell->xf >= workBook->xfs.count)
        return false;

    // Получаем индекс формата (format index) для этой ячейки
    WORD formatIdx = workBook->xfs.xf[cell->xf].format;

    // 2. Проверка стандартных встроенных форматов Excel для Даты/Времени
    // 14: mm-dd-yy, 15: d-mmm-yy, 16: d-mmm, 17: mmm-yy
    // 18: h:mm AM/PM, 19: h:mm:ss AM/PM, 20: h:mm, 21: h:mm:ss, 22: m/d/yy h:mm
    // 45: mm:ss, 46: [h]:mm:ss, 47: mm:ss.0
    if ((formatIdx >= 14 && formatIdx <= 22) || (formatIdx >= 45 && formatIdx <= 47))
        return true;


    // 3. Проверка пользовательских (Custom) форматов
    // Ищем строку формата в таблице форматов книги
    for (DWORD i = 0; i < workBook->formats.count; i++) {
        if (workBook->formats.format[i].index == formatIdx) {
            std::string formatStr(workBook->formats.format[i].value);

            // Переводим в нижний регистр для поиска
            for(unsigned int j = 0; j < formatStr.length(); j++){
                if(formatStr.at(j) >= 'A' && formatStr.at(j) <= 'Z')
                    formatStr[j] += 0x20;
            }

            // Если в маске формата есть символы даты (y, m, d), это дата
            if (formatStr.find('d') != std::string::npos ||
                formatStr.find('m') != std::string::npos ||
                formatStr.find('y') != std::string::npos) {
                return true;
            }
            break;
        }
    }

    return false;
}

xlsString WorkBook::dateTimeToStr(double cellData) const
{
    xlsString ret;

    if((int)cellData != 0){
        //Получаем число суток до 01.01.1972 (в xls дата содержит число суток с 30 декабря 1899 года)
        cellData -= (25569 + 730);
        int cellDate = (int)cellData;

        int dayInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        int month = 0;
        int offsYear;
        int offsQuatrYear = cellDate / 1461;
        cellDate -= offsQuatrYear * 1461;
        if(cellDate < 366){
            //Это високосный год
            dayInMonth[1] = 29;
            offsYear = offsQuatrYear * 4;
        }
        else{
            cellDate -= 366;
            offsYear = offsQuatrYear * 4 + 1 + cellDate / 365;
            cellDate -= (cellDate / 365) * 365;
        }

        for(int i = 0; i < 12; i++){
            if(cellDate < dayInMonth[i]){
                month = i+1;
                break;
            }
            cellDate -= dayInMonth[i];
        }
        int lenDate = 10;
        xlsString date(lenDate, '\0');
        std::snprintf(&date[0], lenDate + 1, "%02d.%02d.%d", cellDate+1, month, 1972 + offsYear);

        ret += date;
    }

    if(cellData - (int)cellData != 0.0){
        int cellTime = (cellData - (int)cellData) * 86400;
        int hour = cellTime / 3600;
        int minute = (cellTime - hour * 3600) / 60;
        int sec = cellTime - hour * 3600 - minute * 60;
        int lenTime = 8;
        xlsString time(lenTime, '\0');
        std::snprintf(&time[0], lenTime + 1, "%02d:%02d:%02d", hour, minute, sec);
        if(ret != "")
            ret += " ";
        ret += time;
    }

    return ret;

}

void WorkBook::FormatCell(xlsCell *cell, cellContent& content) const
{
	uint32_t col = cell->col;

	content.str = char2string(cell->str);
    content.row = cell->row;
	
    content.col = col;
	if(col < 26) {
		content.colStr[0] = 'A' + (char)col;
		content.colStr[1] = '\0';
	} else {
		content.colStr[0] = 'A' + (char)(col/26);
		content.colStr[1] = 'A' + (char)(col%26);
	}
	content.colStr[2] = '\0';

	switch(cell->id) {
    case XLS_RECORD_FORMULA:
		// test for formula, if
        if(cell->l == 0) {
			content.type = cellFloat;
			content.val.d = cell->d;
		} else {
			if(!strcmp((char *)cell->str, "bool")) {
				content.type = cellBool;
				content.val.b = (bool)cell->d;
			} else
			if(!strcmp((char *)cell->str, "error")) {
				content.type = cellError;
				content.val.e = (int32_t)cell->d;
			} else {
				content.type = cellString;
			}
		}
        break;
    case XLS_RECORD_LABELSST:
    case XLS_RECORD_LABEL:
		content.type = cellString;
		content.val.l = cell->l;	// possible numeric conversion done for you
		break;
    case XLS_RECORD_NUMBER:
    case XLS_RECORD_RK:
        if(isDateTime(cell)){
            content.type = cellDateTime;
            content.val.d = cell->d;
            content.str = dateTimeToStr(cell->d);
        }
        else{
            content.type = cellFloat;
            content.val.d = cell->d;
        }
        break;

    default:
		content.type = cellUnknown;
        break;
    }
}


void WorkBook::ShowCell(const cellContent& content) const
{
	const char *name;
	switch(content.type) {
	case cellBlank:		name = "cellBlank";		break;
	case cellString:	name = "cellString";	break;
	case cellInteger:	name = "cellInteger";	break;
	case cellFloat:		name = "cellFloat";		break;
	case cellBool:		name = "cellBool";		break;
	case cellError:		name = "cellError";		break;
	default:			name = "cellUnknown";	break;
	}

	printf("====================\n");
	printf("CellType: %s row=%u col=%s/%u\n", name, content.row, content.colStr, content.col);
	printf("   string:    " XLS_STR_SPEC "\n", content.str.c_str());
	
	switch(content.type) {
	case cellInteger:	printf("     long:    %ld\n", content.val.l);					break;
	case cellFloat:		printf("    float:    %lf\n", content.val.d);					break;
	case cellBool:		printf("     bool:    %s\n", content.val.b ? "true" : "false");	break;
	case cellError:		printf("    error:    %ld\n", content.val.l);					break;
	default: break;
	}
}

xlsString WorkBook::GetSummaryAppName(void) const
{
	return char2string((char *)summary->appName);
}

xlsString WorkBook::GetSummaryAuthor(void) const
{
	return char2string((char *)summary->author);
}

xlsString WorkBook::GetSummaryCategory(void) const
{
	return char2string((char *)summary->category);
}

xlsString WorkBook::GetSummaryComment(void) const
{
	return char2string((char *)summary->comment);
}

xlsString WorkBook::GetSummaryCompany(void) const
{
	return char2string((char *)summary->company);
}

xlsString WorkBook::GetSummaryKeywords(void) const
{
	return char2string((char *)summary->keywords);
}

xlsString WorkBook::GetSummaryLastAuthor(void) const
{
	return char2string((char *)summary->lastAuthor);
}

xlsString WorkBook::GetSummaryManager(void) const
{
	return char2string((char *)summary->manager);
}

xlsString WorkBook::GetSummarySubject(void) const
{
	return char2string((char *)summary->subject);
}

xlsString WorkBook::GetSummaryTitle(void) const
{
	return char2string((char *)summary->title);
}


#if XLS_WIDE_STRINGS == 0

xlsString WorkBook::char2string(const char *ptr) const
{
	return string(ptr);
}

#else

bool WorkBook::isAscii(const char *ptr) const
{
	bool isAscii = false;	
	uint8_t c;
	while((c = *ptr++)) {
		if(c & 0x80) {
			isAscii = true;
			break;
		}
	}
	return isAscii;
}

xlsString WorkBook::char2string(const char *ptr) const
{
	xlsString s;
	size_t len = strlen(ptr);
	size_t wlen = len * sizeof(wchar_t);

	s.reserve(wlen);

	if(isAscii(ptr)) {
		uint8_t c;
		while((c = *ptr++)) {
			s.push_back(c);
		}
	} else {
		wchar_t *wstr = (wchar_t *)calloc(len, sizeof(wchar_t));
		char *inbuf = (char *)ptr;
		char *outbuf = (char *)wstr;
		size_t inbytesleft = len;
		size_t outbytesleft = wlen;
		size_t size = iconv(iconvCD, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
		assert(size != (size_t)(-1));

		wchar_t c;
		while((c = *wstr++)) {
			s.push_back(c);
		}
	}
	return s;
}
#endif

} // namespace
