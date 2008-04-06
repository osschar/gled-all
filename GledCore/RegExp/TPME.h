
////////////////////////////////////////////////////////////////////////////////////////////////
//
// This file is redistributable on the same terms as the PCRE 4.3 license, except copyright
// notice must be attributed to:
//
// (C) 2003 Zachary Hansen xaxxon@slackworks.com
//
// Distribution under the GPL or LGPL overrides any other restrictions, as in the PCRE license
//
////////////////////////////////////////////////////////////////////////////////////////////////
//
// Modified for ROOT's TString type. Matevz Tadel, 9.2005.

#ifndef ROOT_PME_H
#define ROOT_PME_H

#include <TString.h>
#include <vector>

using namespace std;

#include "pcre.h"



/// PME wraps the PCRE C API into a more perl-like syntax
/**
 * PME wraps the PCRE C API into a more perl-liek syntax.  It supports single matching,
 *   global matching (where the regex resumes where it left off on the previous iteration),
 *   single and global substitutions (including using backrefs in the substituted TStrings),
 *   splits based on regex comparisons, and a syntactically easy way to get subTStrings out f
 *   from backrefs and splits.
 */
typedef vector < TString > StringVector;
class TPME
{
public:

	/// default constructor -- virtually worthless
	TPME( );

	/// s is the regular expression, opts are PCRE flags bit-wise or'd together
	TPME(const TString & s, unsigned opts );

	/// s is the regular expression, opts is a perl-like TString of modifier letters "gi" is global and case insensitive
	TPME(const TString & s, const TString & opts = "");

	/// s is the regular expression, opts are PCRE flags bit-wise or'd together
	TPME(const char * s, unsigned opts );

	/// s is the regular expression, opts is a perl-like TString of modifier letters "gi" is global and case insensitive
	TPME ( const char * s, const TString & opts = "" );

	/// TPME copy constructor
	TPME(const TPME & r);

	/// TPME = TPME assignment operator
	const TPME & operator = (const TPME & r);

	/// destructor
	~TPME();

	/// stores results from matches
	typedef pair<int, int> markers;

	/// returns options set on this object
	unsigned				options();

	/// sets new options on the object -- doesn't work?
	void					options(unsigned opts);

	/// runs a match on s against the regex 'this' was created with -- returns the number of matches found
	int         			match(const TString & s, unsigned offset = 0);

	/// splits s based on delimiters matching the regex.
	int         			split(const TString & s, ///< TString to split on
					      unsigned maxfields = 0 ///< maximum number of fields to be split out.  0 means split all fields, but discard any trailing empty bits.  Negative means split all fields and keep trailing empty bits.  Positive means keep up to N fields including any empty fields less than N.  Anything remaining is in the last field.
);

	/// substitutes out whatever matches the regex for the second paramter
	TString             sub ( const TString & s,
				  const TString & r,
				  int dodollarsubstitution = 1 );

	/// study the regular expression to make it faster
	void                    study();

	/// returns the subTString from the internal m_marks vector requires having run match or split first
	TString             operator[](int);

	/// resets the regex object -- mostly useful for global matching
	void                    reset();

	/// returns the number of back references returned by the last match/sub call
	int                     NumBackRefs ( ) { return nMatches; }

	/// whether this regex is valid
	int IsValid ( ) { return nValid; }

	/// returns a vector of TStrings for the last match/split/sub
	StringVector GetStringVector ( );


protected:

	/// used internally for operator[]
	/** \deprecated going away */
	TString		       	substr(const TString & s,
				       const vector<markers> & marks, unsigned index);


	pcre * re; ///< pcre structure from pcre_compile

	unsigned _opts; ///< bit-flag options for pcre_compile

	pcre_extra * extra;	///< results from pcre_study

	int nMatches; ///< number of matches returned from last pcre_exec call

	vector<markers> m_marks; ///< last set of indexes of matches

	TString laststringmatched; ///< copy of the last TString matched
	void * addressoflaststring; ///< used for checking for change of TString in global match

	int m_isglobal; ///< non-pcre flag for 'g' behaviour
	int lastglobalposition; ///< end of last match when m_isglobal != 0

	/// compiles the regex -- automatically called on construction
	void compile(const TString & s);

	/// used to make a copy of a regex object
	static pcre * clone_re(pcre * re);

	/// takes perl-style character modifiers and determines the corresponding PCRE flags
	unsigned int DeterminePcreOptions ( const TString & opts = "" );

	/// deals with $1-type constructs in the replacement TString in a substitution
	TString UpdateReplacementString ( const TString & r );


	/// flag as to whether this regex is valid (compiled without error)
	int                     nValid;

};


#endif // TPME_H
