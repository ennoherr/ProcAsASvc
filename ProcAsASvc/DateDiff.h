#pragma once

class CDateDiff
{
public:
	long long int DiffDays(int day1, int month1, int year1, int day2, int month2, int year2);

private:
	int IsLeapYear(int year);
	int DaysOfMonth(int month, int year);


};

