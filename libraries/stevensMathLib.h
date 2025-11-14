/**
 * stevensMathLib.h
 * 
 * Defines the stevensMathLib.h object, an object containing useful functions for interacting with numbers in C++.
 */
#include<algorithm>
#include<cmath>
#include <limits>


namespace stevensMathLib
{
	/**
	 * @brief Use srand() to seed random number generation. This is ideally called before you generate any random numbers
	 * 		  so that they come out as random.
	*/
	void seedRNG()
	{
		//Seed all of the random numbers we'll be generating
		srand((unsigned) time(NULL));
	}


	/*
	** Rounds a number to the nearest 10th. If we want to return the actual
	** numbers, we will need to refer to one of the alternative options at this link, as
	** we don't know if a number will return as an integer or a float until runtime:
	** https://stackoverflow.com/questions/17649136/function-which-is-able-to-return-different-types
	** 
	*/
	float roundToNearest10th(float numberToRound)
	{
		if (floor(numberToRound) == numberToRound) //Check to see if a number has no decimal part. If a cost has no decimal part, then we print the number as an integer.
		{
			return static_cast<int>(numberToRound);
		}
		else //If the number has a decimal part, we round the number to the nearest tenth so we don't see a bunch of trailing zeroes
		{
			float roundedNumber = floor(numberToRound*10+0.5)/10;
			if (floor(roundedNumber) == roundedNumber) //We doublecheck here to make sure that we haven't rounded down to .00... If we have, then we print the number as an integer.
			{
				return static_cast<int>(roundedNumber);
			}
			else
			{
				return roundedNumber;
			}
		}
	}


	/**
	 * @brief Rounds a number to a as many points of percision as we indicate.
	 * 
	 * @credit https://stackoverflow.com/a/3237340/16511184
	 * 
	 * @param value The value we wish to round
	 * @param precision The number of places to the right of the decimal point we wish to round the value to.
	 * 
	 * @return The parameter value rounded to the number of decimal places indicated by the parameter precision.
	 */
	float round( float value, int precision )
	{
		const int adjustment = pow(10,precision);
		return std::round( value*(adjustment) + 0.5 )/adjustment;
	}


	/**
	 *	Generates a random floating point number from a given lower bound to a given upper bound.
		*
		* 	Parameters:
		* 		float lowerBound - Inclusive lower bound for generating a random float, the minimum number that can be generated.
		* 		float upperBound - Inclusive upper bound, the maximum number that can be generated.
		* 
		* 	Returns: 
		* 		float - A random floating point number between the lower bound and the upper bound.
	*/
	float randomFloat(	float lowerBound = 0.0,
						float upperBound = 1.0	)
	{
		return (lowerBound + static_cast <float> (rand() /( static_cast <float> (RAND_MAX/(upperBound-lowerBound)))));
	}


	/**
	 * Generates a random integer number from a given lower bound to a given upper bound.
	 * 
	 * Parameters:
	 * 		int lowerBound - Inclusive lower bound for generating an integer, the minimum number that can be generated.
	 * 		int upperBound - Exclusive upper bound for generating an integer. The one above the maximum number that can be generated.
	 * 
	 * Returns:
	 * 		int - A random integer between the lower bound and the upper bound.
	*/
	int randomInt(	int lowerBound = 0,
					int upperBound = RAND_MAX)
	{
		// Handle edge case where upperBound <= lowerBound
		if (upperBound <= lowerBound) {
			return lowerBound;
		}
		return (lowerBound + (rand() % (upperBound - lowerBound)));
	}


	/**
	 * @brief Generates a random integer that is not equal to any of the integers given in a vector.
	 * 
	 * @param integerBlacklist A vector containing integers that you do not want randomly returned from this function.
	 * @param lowerBound Inclusive lower bound for generating an integer. The minimum number that can be generated.
	 * @param upperBound Exclusive upper bound for generating an integer. Limit at which numbers can be generated up to.
	 * 
	 * @return An integer in a requested range not equal to any of the integers provided in the integerBlacklist parameter.
	 */
	int randomIntNotInVec(	const std::vector<int> & integerBlacklist,
							int lowerBound = 0,
							int upperBound = RAND_MAX	)
	{
		//Create a vector containing all of non-blacklist integers in the range
		std::vector<int> integerVec;
		for(int i = lowerBound; i < upperBound; i++)
		{
			if(std::find(integerBlacklist.begin(), integerBlacklist.end(), i) == integerBlacklist.end())
			{
				integerVec.push_back(i);
			}
		}

		//Get a random integer from the integerVec we pushed all valid ints back into
		//*poached from stevensVectorLib
		if(integerVec.size() == 0)
		{
			throw std::invalid_argument("stevensMathLib:randomIntNotInVec() - No valid integers in the range remain after considering integerBlacklist.");
		}
		int randomPos = std::rand() % integerVec.size();
		return integerVec[randomPos];
	}


	/**
	 * Converts a floating point number to an integer.
	 * Taken from: https://stackoverflow.com/a/2545218
	 * 
	 * 
	 * 
	*/
	template <class FloatType>
	int floatToInt(const FloatType &num) 
	{
		//check if float fits into integer
		if ( std::numeric_limits<int>::digits < std::numeric_limits<FloatType>::digits)
		{
			// check if float is smaller than max int
			if( (num < static_cast<FloatType>( std::numeric_limits<int>::max())) &&
				(num > static_cast<FloatType>( std::numeric_limits<int>::min())) ) 
			{
				return static_cast<int>(num); //safe to cast
			} 
			else
			{
				//throw bad_cast;
				std::cerr << "Unsafe conversion of value:" << num << std::endl;
				//NaN is not defined for int return the largest int value
				return std::numeric_limits<int>::max();
			}
		} 
		else 
		{
			//It is safe to cast
			return static_cast<int>(num);
		}
	}


	/**
	 * TODO: How to make this only work for numerical types?
	 * 
	 * Given a lower bound and upper bound of some numerical type, check to see if a variable in within the range specified by the upper and lower bounds.
	 * If it is, return true. Otherwise, return false.
	 * 
	 * Parameters:
	 *  const T checkValue - The value which we are checking to see if it falls between the bounds
	 *  const T lowerBound - The smaller value we are checking to see if our value is greater than/greater than or equal to.
	 *  const T upperBound - The bigger  value we are checking to see if our value is less than/less than or equal to.
	 *  std::string boundSetting - The way which we are treating the edge cases of our range checking. Possible values are:
	 *                                  "exclusive" - Bounds are checked using > and < signs
	 *                                  "inclusive" - Bounds are checked using >= and <= signs
	 * 
	 * Returns:
	 *  bool - True if the checkValue falls within the bounds, False otherwise.
	*/
	template<typename T>
	bool in_range(  const T checkValue,
					const T lowerBound,
					const T upperBound,
					std::string boundSetting = "inclusive"  )
	{	
		if (boundSetting == "inclusive")
		{
			if((checkValue >= lowerBound) && (checkValue <= upperBound))
			{
				return true;
			}
		}
		else if(boundSetting == "exclusive")
		{
			if((checkValue > lowerBound) && (checkValue < upperBound))
			{
				return true;
			}
		}
		else
		{
			std::cerr << "StevensMathLib Error: in_range: Could not recognize the boundSetting parameter '" << boundSetting << "'." << std::endl;
		}

		return false;
	}
};