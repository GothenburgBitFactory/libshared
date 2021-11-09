////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2013 - 2021, Göteborg Bit Factory.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// http://www.opensource.org/licenses/mit-license.php
//
////////////////////////////////////////////////////////////////////////////////

#include <cmake.h>
#include <test.h>
#include <Datetime.h>

////////////////////////////////////////////////////////////////////////////////
void testInit (UnitTest& t, const std::string& value, Datetime& var)
{
  try
  {
    var = Datetime (value);
    t.pass (value + " --> valid");
  }

  catch (const std::string& e)
  {
    t.fail (value + " --> valid");
    t.diag (e);
  }

  catch (...)
  {
    t.fail (value + " --> valid");
  }
}

////////////////////////////////////////////////////////////////////////////////
int main (int, char**)
{
  UnitTest t (164);

  Datetime sunday;    testInit (t, "sunday",    sunday);
  Datetime monday;    testInit (t, "monday",    monday);
  Datetime tuesday;   testInit (t, "tuesday",   tuesday);
  Datetime wednesday; testInit (t, "wednesday", wednesday);
  Datetime thursday;  testInit (t, "thursday",  thursday);
  Datetime friday;    testInit (t, "friday",    friday);
  Datetime saturday;  testInit (t, "saturday",  saturday);

  Datetime sun; testInit (t, "sun", sun);
  Datetime mon; testInit (t, "mon", mon);
  Datetime tue; testInit (t, "tue", tue);
  Datetime wed; testInit (t, "wed", wed);
  Datetime thu; testInit (t, "thu", thu);
  Datetime fri; testInit (t, "fri", fri);
  Datetime sat; testInit (t, "sat", sat);

  t.ok (sunday    == sun, "sunday == sun");
  t.ok (monday    == mon, "monday == mon");
  t.ok (tuesday   == tue, "tuesday == tue");
  t.ok (wednesday == wed, "wednesday == wed");
  t.ok (thursday  == thu, "thursday == thu");
  t.ok (friday    == fri, "friday == fri");
  t.ok (saturday  == sat, "saturday == sat");

  Datetime sundayCapital;    testInit (t, "Sunday",    sundayCapital);
  Datetime mondayCapital;    testInit (t, "Monday",    mondayCapital);
  Datetime tuesdayCapital;   testInit (t, "Tuesday",   tuesdayCapital);
  Datetime wednesdayCapital; testInit (t, "Wednesday", wednesdayCapital);
  Datetime thursdayCapital;  testInit (t, "Thursday",  thursdayCapital);
  Datetime fridayCapital;    testInit (t, "Friday",    fridayCapital);
  Datetime saturdayCapital;  testInit (t, "Saturday",  saturdayCapital);

  t.ok (sundayCapital    == sun, "Sunday == sun");
  t.ok (mondayCapital    == mon, "Monday == mon");
  t.ok (tuesdayCapital   == tue, "Tuesday == tue");
  t.ok (wednesdayCapital == wed, "Wednesday == wed");
  t.ok (thursdayCapital  == thu, "Thursday == thu");
  t.ok (fridayCapital    == fri, "Friday == fri");
  t.ok (saturdayCapital  == sat, "Saturday == sat");

  Datetime january;   testInit (t, "january",   january);
  Datetime february;  testInit (t, "february",  february);
  Datetime march;     testInit (t, "march",     march);
  Datetime april;     testInit (t, "april",     april);
  Datetime may;       testInit (t, "may",       may);
  Datetime june;      testInit (t, "june",      june);
  Datetime july;      testInit (t, "july",      july);
  Datetime august;    testInit (t, "august",    august);
  Datetime september; testInit (t, "september", september);
  Datetime october;   testInit (t, "october",   october);
  Datetime november;  testInit (t, "november",  november);
  Datetime december;  testInit (t, "december",  december);

  Datetime jan; testInit (t, "jan", jan);
  Datetime feb; testInit (t, "feb", feb);
  Datetime mar; testInit (t, "mar", mar);
  Datetime apr; testInit (t, "apr", apr);
  Datetime jun; testInit (t, "jun", jun);
  Datetime jul; testInit (t, "jul", jul);
  Datetime aug; testInit (t, "aug", aug);
  Datetime sep; testInit (t, "sep", sep);
  Datetime oct; testInit (t, "oct", oct);
  Datetime nov; testInit (t, "nov", nov);
  Datetime dec; testInit (t, "dec", dec);

  t.ok (january   == jan, "january == jan");
  t.ok (february  == feb, "february == feb");
  t.ok (march     == mar, "march == mar");
  t.ok (april     == apr, "april == apr");
  // May has only three letters.
  t.ok (june      == jun, "june == jun");
  t.ok (july      == jul, "july == jul");
  t.ok (august    == aug, "august == aug");
  t.ok (september == sep, "september == sep");
  t.ok (october   == oct, "october == oct");
  t.ok (november  == nov, "november == nov");
  t.ok (december  == dec, "december == dec");

  Datetime januaryCapital;   testInit (t, "January",   januaryCapital);
  Datetime februaryCapital;  testInit (t, "February",  februaryCapital);
  Datetime marchCapital;     testInit (t, "March",     marchCapital);
  Datetime aprilCapital;     testInit (t, "April",     aprilCapital);
  Datetime mayCapital;       testInit (t, "May",       mayCapital);
  Datetime juneCapital;      testInit (t, "June",      juneCapital);
  Datetime julyCapital;      testInit (t, "July",      julyCapital);
  Datetime augustCapital;    testInit (t, "August",    augustCapital);
  Datetime septemberCapital; testInit (t, "September", septemberCapital);
  Datetime octoberCapital;   testInit (t, "October",   octoberCapital);
  Datetime novemberCapital;  testInit (t, "November",  novemberCapital);
  Datetime decemberCapital;  testInit (t, "December",  decemberCapital);

  t.ok (januaryCapital   == jan, "January == jan");
  t.ok (februaryCapital  == feb, "February == feb");
  t.ok (marchCapital     == mar, "March == mar");
  t.ok (aprilCapital     == apr, "April == apr");
  // May has only three letters.
  t.ok (juneCapital      == jun, "June == jun");
  t.ok (julyCapital      == jul, "July == jul");
  t.ok (augustCapital    == aug, "August == aug");
  t.ok (septemberCapital == sep, "September == sep");
  t.ok (octoberCapital   == oct, "October == oct");
  t.ok (novemberCapital  == nov, "November == nov");
  t.ok (decemberCapital  == dec, "December == dec");

  // Simply instantiate these for now.  Test later.
  Datetime now;            testInit (t, "now", now);
  Datetime yesterday;      testInit (t, "yesterday", yesterday);
  Datetime today;          testInit (t, "today", today);
  Datetime tomorrow;       testInit (t, "tomorrow", tomorrow);
  Datetime first;          testInit (t, "1st", first);
  Datetime second;         testInit (t, "2nd", second);
  Datetime third;          testInit (t, "3rd", third);
  Datetime fourth;         testInit (t, "4th", fourth);
  Datetime later;          testInit (t, "later", later);
  Datetime someday;        testInit (t, "someday", someday);
  Datetime sopd;           testInit (t, "sopd", sopd);
  Datetime sond;           testInit (t, "sond", sond);
  Datetime sod;            testInit (t, "sod", sod);
  Datetime eopd;           testInit (t, "eopd", eopd);
  Datetime eond;           testInit (t, "eond", eond);
  Datetime eod;            testInit (t, "eod", eod);
  Datetime sopw;           testInit (t, "sopw", sopw);
  Datetime sonw;           testInit (t, "sonw", sonw);
  Datetime sow;            testInit (t, "sow", sow);
  Datetime eopw;           testInit (t, "eopw", eopw);
  Datetime eonw;           testInit (t, "eonw", eonw);
  Datetime eow;            testInit (t, "eow", eow);
  Datetime sopww;          testInit (t, "sopww", sopww);
  Datetime sonww;          testInit (t, "sonww", sonww);
  Datetime soww;           testInit (t, "soww", soww);
  Datetime eopww;          testInit (t, "eopww", eopww);
  Datetime eonww;          testInit (t, "eonww", eonww);
  Datetime eoww;           testInit (t, "eoww", eoww);
  Datetime sopm;           testInit (t, "sopm", sopm);
  Datetime sonm;           testInit (t, "sonm", sonm);
  Datetime som;            testInit (t, "som", som);
  Datetime eopm;           testInit (t, "eopm", eopm);
  Datetime eonm;           testInit (t, "eonm", eonm);
  Datetime eom;            testInit (t, "eom", eom);
  Datetime sopq;           testInit (t, "sopq", sopq);
  Datetime sonq;           testInit (t, "sonq", sonq);
  Datetime soq;            testInit (t, "soq", soq);
  Datetime eopq;           testInit (t, "eopq", eopq);
  Datetime eonq;           testInit (t, "eonq", eonq);
  Datetime eoq;            testInit (t, "eoq", eoq);
  Datetime sopy;           testInit (t, "sopy", sopy);
  Datetime sony;           testInit (t, "sony", sony);
  Datetime soy;            testInit (t, "soy", soy);
  Datetime eopy;           testInit (t, "eopy", eopy);
  Datetime eony;           testInit (t, "eony", eony);
  Datetime eoy;            testInit (t, "eoy", eoy);
  Datetime easter;         testInit (t, "easter", easter);
  Datetime eastermonday;   testInit (t, "eastermonday", eastermonday);
  Datetime ascension;      testInit (t, "ascension", ascension);
  Datetime pentecost;      testInit (t, "pentecost", pentecost);
  Datetime goodfriday;     testInit (t, "goodfriday", goodfriday);

  Datetime midsommar;      testInit (t, "midsommar", midsommar);
  Datetime midsommarafton; testInit (t, "midsommarafton", midsommarafton);
  Datetime juhannus;       testInit (t, "juhannus", juhannus);

  // Check abbreviations.
  // TW-1515: abbreviation.minimum does not apply to date recognition
  Datetime yesterday2;      testInit (t, "yesterday", yesterday2);
  Datetime yesterday3;      testInit (t, "yesterda",  yesterday3);
  Datetime yesterday4;      testInit (t, "yesterd",   yesterday4);
  Datetime yesterday5;      testInit (t, "yester",    yesterday5);
  Datetime yesterday6;      testInit (t, "yeste",     yesterday6);
  Datetime yesterday7;      testInit (t, "yest",      yesterday7);
  Datetime yesterday8;      testInit (t, "yes",       yesterday8);

  t.ok (eod < tomorrow,             "eod  < tomorrow");
  t.ok (eom < sonm,                 "eom  < sonm");
  t.ok (eond > tomorrow,            "eond > tomorrow");
  t.ok (eonm > sonm,                "eonm > sonm");
  t.ok (eonq > sonq,                "eonq > sonq");
  t.ok (eonw > sonw,                "eonw > sonw");
  t.ok (eonww > sonww,              "eonww > sonww");
  t.ok (eony > sony,                "eony > sony");
  t.ok (eopd < today,               "eopd < today");
  t.ok (eopm < som,                 "eopm < som");
  t.ok (eopq < soq,                 "eopq < soq");
  t.ok (eopw < sow,                 "eopw < sow");
  t.ok (eopww < soww,               "eopww < soww");
  t.ok (eopy < soy,                 "eopy < soy");
  t.ok (eoq == eoq,                 "eoq == eoq");
  t.ok (eoq < sonq,                 "eoq < sonq");
  t.ok (eow == eow,                 "eow == eow");
  t.ok (eow < sonw,                 "eow < sonw");
  t.ok (eoww >= eopww,              "eoww >= eopww");
  t.ok (eoy == eoy,                 "eoy == eoy");
  t.ok (eoy < sony,                 "eoy < sony");
  t.ok (later == someday,           "later == someday");
  t.ok (now < later,                "now < later");
  t.ok (now < someday,              "now < someday");
  t.ok (now >= today,               "now >= today");
  t.ok (sod == today,               "sod == today");
  t.ok (som < eom,                  "som < eom");
  t.ok (sond == tomorrow,           "sond == tomorrow");
  t.ok (sonq > eoq,                 "sonq > eoq");
  t.ok (sonq > soq,                 "sonq > soq");
  t.ok (sonw > eow,                 "sonw > eow");
  t.ok (sony > eoy,                 "sony > eoy");
  t.ok (sony > soy,                 "sony > soy");
  t.ok (sopd == yesterday,          "sopd == yesterday");
  t.ok (sopm < som,                 "sopm < som");
  t.ok (sopq < soq,                 "sopq < soq");
  t.ok (sopw < sow,                 "sopw < sow");
  t.ok (sopww < soww,               "sopww < soww");
  t.ok (sopy < soy,                 "sopy < soy");
  t.ok (sow < sonw,                 "sow < sonw");
  t.ok (soww < sonww,               "soww < sonww");
  t.ok (soy > sopy,                 "soy > sopy");
  t.ok (today < tomorrow,           "today < tomorrow");
  t.ok (yesterday < today,          "yesterday < today");
  t.ok (goodfriday < easter,        "goodfriday < easter");
  t.ok (easter < eastermonday,      "easter < eastermonday");
  t.ok (midsommarafton < midsommar, "midsommarafton < midsommar");
  t.ok (juhannus == midsommarafton, "juhannus == midsommarafton");

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
