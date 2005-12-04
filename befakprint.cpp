
// TODO:
//	- wlaczyc slownie do klasy?
//
// ogolne:
//		wygenerowanie podsumowania z rozbiciem na stawki
//		obliczenie sumy do zaplaty
// text:
//		generowanie tabelki
//			szerokosci na sztywno dla 80/136
//			truncate wszystko
//		RAZEM osobno, zeby moglo zajsc na sume/slownie
//


#include "befakprint.h"

#include <stdio.h>

// baseclass with stuff, inherit to print/export/whatever

beFakPrint::beFakPrint(int id, sqlite *db) {

	dbData = db;
	fakturaid = id;
	if (id<1) {
		printf("illegal id!\n");
		return;
	}
	// readout stuff
printf("reading stuff for id=%i\n",fakturaid);
	fdata = new fakturadat(db);
	flist = new pozfaklist(db);

	fdata->id = fakturaid;
	fdata->fetch();
	flist->fetch(fdata->id);

	// XXX duplicated in dialfirma constructor!!! separate!!!
	int i, j;
	int nRows, nCols;
	char **result;
	BString sql;
	sql = "SELECT ";
	sql += "nazwa, adres, kod, miejscowosc, telefon, email";
	sql += ", nip, regon, bank, konto";
	sql += " FROM konfiguracja WHERE zrobiona = 1";
//printf("sql:%s\n",sql.String());
	sqlite_get_table(dbData, sql.String(), &result, &nRows, &nCols, &dbErrMsg);
//printf ("got:%ix%i\n", nRows, nCols);
	if (nRows < 1) {
		// XXX brak informacji o własnej firmie, co robić?
	} else {
//		printf("not initial\n");
		// readout data
		i = nCols;
		own[0] = result[i++];
		for (j=2;j<=10;j++) {
			own[j] = result[i++];
		}
	}
	printf("stuff in memory, do sth with it\n");
}

beFakPrint::~beFakPrint() {
printf("at the end call destructor from baseclass\n");
	delete flist;
	delete fdata;
}

const char *setki[] = { "", "sto ", "dwieście ", "trzysta ", "czterysta ", "pięćset ", "sześćset ", "siedemset ", "osiemset ", "dziewięćset " };
const char *dziesiatki[] = { "", "", "dwadzieścia ", "trzydzieści ", "czterdzieści ", "pięćdziesiąt ", "sześćdziesiąt", "siedemdziesiąt ", "osiemdziesiąt ", "dziewięćdziesiąt " };
const char *nascie[] = { "dziesięć ", "jedenaście ", "dwanaście ", "trzynaście ", "czternaście ", "piętnaście ", "szesnaście ", "siedemnaście ", "osiemnaście ", "dziewiętnaście " };
const char *jednosci[] = { "", "jeden ", "dwa ", "trzy ", "cztery ", "pięć ", "sześć ", "siedem ", "osiem ", "dziewięć " };

const char *rozbij_tysiac(int val) {
	static BString tmp;
	int i;
	int t = val;

	tmp = "";
	i = t / 100;
	tmp += setki[i];
	t = t % 100;
	i = t / 10;
	t = t % 10;
	if (i!=1) {
		tmp += dziesiatki[i];
		tmp += jednosci[t];
	} else {
		tmp += nascie[t];
	}
	return tmp.String();
}

const char *slownie(const char *input) {
	static BString tmp;

	int i = 0;
	int l = strlen(input);
	char c;

	int z = 0;		// zlote
	int g = 0;		// grosze
	int w = 0;		// wykladnik
	bool grosze = false;

	while (i<l) {
		c = input[i++];
		if (c == '.')
			grosze = true;
		else
		if (!grosze)
			z = z*10+c-'0';
		else {
			w++;
			if (w<3)
				g=g*10+c-'0';
		}
	}
	if (w==1) g*=10;

	i = z / 1000000;
	if (i!=0) {
		tmp += rozbij_tysiac(i);
		tmp += "mln. ";
	}
	z = z % 1000000;

	i = z / 1000;
	if (i!=0) {
		tmp += rozbij_tysiac(i);
		tmp += "tys. ";
	}
	z = z % 1000;

	i = z;
	if (i!=0) {
		tmp += rozbij_tysiac(i);
	}
	if ((tmp.Length()==0) && (g==0))
		tmp = "zero ";
	if (tmp.Length()!=0)
		tmp += "zł ";

	i = g;
	if (i!=0) {
		tmp += rozbij_tysiac(i);
		tmp += "gr";
	}

	return tmp.String();
}

//----------------------
// sample implementation

#define NCOLS	80		// liczba znaków w wierszu tekstowym
#define ELINE	"\n"	// znak końca linii

// helpers
const char *rightAlign(const BString line, const BString right);
const char *centerAlign(const BString line);
const char *halfAlign(const BString line, const BString right);

void beFakPrint::Go(void) {
	printf("override and do sth with data, then killyourself\n");
// text dump
// lewy górny: tabliczka firmy, prawy 1 wiersz: miejsce, data wydania
BString tmp, out, line;
	out = ""; line="", tmp = "";
	printf("--------------------\n");
	//[1] nazwasprzedawcy .... miejscewyst,datawyst
	line = own[0].String();
	tmp = fdata->ogol[0].String(); tmp += ", "; tmp += fdata->ogol[2];
	line = rightAlign(line, tmp);
	line += ELINE;
	out += line;
	//[2] kodsprz miejscesprz, adres sprz
	line = own[3].String(); line += " "; line += own[4].String();
	line += ", "; line += own[2].String(); line += ELINE;
	out += line;
	//[3] telsprz,  emailsprz
	line = "tel. "; line += own[5].String(); line += ", ";
	line += own[6].String(); line += ELINE;
	out += line;
	//[4] banksprz kontosprz
	line = own[9].String(), line += " "; line += own[10].String(); line += ELINE;
	out += line;
	//[5] [REGON: XXX][ NIP XXX]
	line = "";
	if (own[8].Length()>0) { line += "REGON: "; line += own[8].String(); }
	if (own[7].Length()>0) { if (line.Length()>0) line += ", ";
		line += "NIP: "; line += own[7].String();
	}
	if (line.Length()>0) line += ELINE;
	out += line;
	//[6] [wolna/missing]
	out += ELINE;
	//[7] [wolna]
	out += ELINE;
	//[8] ...Faktura VAT nr XXX....
	line = "Faktura VAT nr "; line += fdata->nazwa.String();
	line = centerAlign(line); line += ELINE;
	out += line;
	//[9] ...[typ faktury]...
	switch (typ) {
		case 2:
			line = "Duplikat";
			break;
		case 1:
			line = "Kopia";
			break;
		case 0:
		default:
			line = "Oryginał";
			break;
	}
	line = centerAlign(line); line += ELINE;
	out += line;
	//[10] [wolna]
	out += ELINE;
	//[11] Nabywca: [nabywca]
	line = "Nabywca: "; line += fdata->odata[0].String(); line += ELINE;
	out += line;
	//[12] Adres: [adresnab], [kodnab] [miejscnab]
	line = "  Adres: "; line += fdata->odata[2].String(); line += ", "; line += fdata->odata[3]; line += " "; line += fdata->odata[4]; line += ELINE;
	out += line;
	//[13]        [telnab], [emailnab]
	line = "         tel. "; line += fdata->odata[5].String(); line += ", "; line += fdata->odata[6]; line += ELINE;
	out += line;
	//[13]        [NIP: [nipnab]], [REGON: [regonab]]
	line = "         ";
	if (fdata->odata[8].Length()>0) { line += "REGON: "; line += fdata->odata[8].String(); }
	if (fdata->odata[7].Length()>0) { if (line.Length()>0) line += ",  ";
		line += "NIP: "; line += fdata->odata[7].String();
	}
	if (line.Length()>0) line += ELINE;
	out += line;
	//[14] [wolna]
	out += ELINE;
	//[15] sposob zaplaty [sposob]...termin zaplaty [termin]
	line = "Sposób zapłaty: "; line += fdata->ogol[5];
	tmp  = "Termin zapłaty: "; tmp += fdata->ogol[6];
	line = halfAlign(line, tmp);
	line += ELINE;
	out += line;
	//[16] data sprzedazy [data]...srodek transportu [srodek]
	line = "Data sprzedaży: "; line += fdata->ogol[3];
	tmp  = "Środek transp.: "; tmp += fdata->ogol[4];
	line = halfAlign(line, tmp);
	line += ELINE;
	out += line;
	//[17] [wolna]
	out += ELINE;
	// [tabela], [podsumowanie]

	//[] [wolna]
	out += ELINE;
	//[] Do zapłaty: [kwota], lub polaczone z RAZEM |
	//[] Słownie: [kwota]     lib polaczone z +---+---+ pod razem
	//[] uwagi: [uwagi, multiline, wrap!]
	//[] wystawil: [wystawil]...odebral:
	line = "    wystawił: "; line += fdata->ogol[1];
	tmp  = "odebrał: ";
	line = halfAlign(line, tmp);
	line += ELINE;
	out += line;
	//[]           ----------           --------------
	line = "              ----------------------";
	tmp  =      "         ----------------------";
	line = halfAlign(line, tmp);
	line += ELINE;
	out += line;
	//[]       podpis osoby upow.       podpis osoby upow.
	line = "                podpis osoby upow.";
	tmp  =      "           podpis osoby upow.";
	line = halfAlign(line, tmp);
	line += ELINE;
	out += line;
	//[] wolna
	out += ELINE;
	printf("%s",out.String());
	printf("%s\n", slownie("15234.34"));
	printf("---------------------\n");
}

const char *rightAlign(const BString line, const BString right) {
	static BString tmp;
	int j;

	tmp = line;
	j = line.CountChars() + right.CountChars();
	if (j < NCOLS) {
		j = NCOLS-j;
		while (j>0) { tmp += " "; j--; };
	}
	tmp += right;
	return tmp.String();
}

const char *centerAlign(const BString line) {
	static BString tmp;
	int j;

	tmp = "";
	j = NCOLS/2 - line.CountChars()/2;
	while (j>0) { tmp += " "; j--; }
	tmp += line;
	return tmp.String();
}

const char *halfAlign(const BString line, const BString right) {
	static BString tmp;
	int j;

	tmp = line;
	j = NCOLS/2 - line.CountChars();
	while (j>0) { tmp += " "; j--; }
	tmp += right;

	return tmp.String();
}
