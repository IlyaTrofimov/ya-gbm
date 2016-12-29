double get_regression(double x[])
{
   double value = 0;

	if (x[12] < 5.570000)
		if (x[12] < 3.920000)
			if (x[10] < 15.200000)
				value += 47.970000;
			else
				value += 37.850000;
		else
			if (x[10] < 18.400000)
				if (x[11] < 376.750000)
					value += 28.716667;
				else
					value += 35.700000;
			else
				value += 26.716667;
	else
		if (x[12] < 14.690000)
			if (x[12] < 9.880000)
				if (x[10] < 14.700000)
					value += 34.240000;
				else
					if (x[10] < 21.000000)
						if (x[10] < 18.700000)
							if (x[10] < 17.400000)
								if (x[10] < 16.100000)
									value += 25.566667;
								else
									value += 23.266667;
							else
								if (x[12] < 7.700000)
									value += 29.976923;
								else
									value += 25.620000;
						else
							value += 23.923333;
					else
						value += 20.066667;
			else
				if (x[12] < 12.340000)
					if (x[10] < 17.600000)
						value += 23.036364;
					else
						value += 21.067568;
				else
					if (x[11] < 378.950000)
						value += 18.992857;
					else
						if (x[11] < 388.650000)
							value += 22.416667;
						else
							value += 20.385714;
		else
			if (x[10] < 20.100000)
				value += 18.672727;
			else
				if (x[12] < 22.740000)
					if (x[12] < 20.340000)
						if (x[11] < 131.420000)
							value += 13.522222;
						else
							value += 15.867442;
					else
						value += 13.511111;
				else
					value += 10.472727;

    return value;
}
