/*
  EHS is a library for adding web server support to a C++ application
  Copyright (C) 2001, 2002 Zac Hansen
  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; version 2
  of the License only.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
  
  Zac Hansen ( xaxxon@slackworks.com )
*/


#include <string>
#include <set>

#include <ehs/ehs.h>

class FormTester : public EHS
{
public:
  typedef std::set<std::string> sString_t;
  typedef sString_t::iterator   sString_i;

  FormTester() { oNameList.insert("QuartaLaTanga"); }

  ResponseCode HandleRequest(HttpRequest  *ipoHttpRequest,
			     HttpResponse *ipoHttpResponse);
	
  sString_t oNameList;
};


// creates a page based on user input -- either displays data from
//   form or presents a form for users to submit data.
ResponseCode FormTester::HandleRequest(HttpRequest  *ipoHttpRequest,
				       HttpResponse *ipoHttpResponse)
{
  char psHtml[5000];

  // if we got data from the user, show it
  if (ipoHttpRequest->FormValues("user").m_sBody.length() ||
      ipoHttpRequest->FormValues("existinguser").m_sBody.length())
  {
    std::string sName;

    sName = ipoHttpRequest->FormValues("existinguser").m_sBody;
    if (ipoHttpRequest->FormValues("user").m_sBody.length())
    {
      sName = ipoHttpRequest->FormValues("user").m_sBody;
    }

    fprintf(stderr, "Got name of %s\n", sName.c_str());

    char *psHtml = new char[5000];
    sprintf(psHtml, "<html><head><title>StringList</title></head>\n<body>Hi %s</body></html>", sName.c_str ( ) );
    oNameList.insert(sName);

    ipoHttpResponse->SetBody(psHtml, strlen(psHtml));
    return HTTPRESPONSECODE_200_OK;
  }
  else
  {
    // otherwise, present the form to the user to fill in
    fprintf ( stderr, "Got no form data\n" );

    // create the options for the dropdown box
    char psOptions [ oNameList.size ( ) * 200 ];
    psOptions [ 0 ] = '\0';

    for (sString_i oCurrentName = oNameList.begin();
	 oCurrentName != oNameList.end();
	 oCurrentName++)
    {
      char psOption[200];
      sprintf(psOption, "<option>%s</option>\n",
	      oCurrentName->substr(0, 150).c_str());
      strcat(psOptions, psOption);
    }

    sprintf(psHtml, "<html><head><title>StringList</title></head> <body>Please enter user name<P> <form action = \"/\" method=GET> User name: <input type = text  name = user><BR> <select name = existinguser width = 20> %s </select> <input type=\"submit\" value=\"Log in\"> </form>\n", 
	    psOptions);

    ipoHttpResponse->SetBody(psHtml, strlen(psHtml));

    return HTTPRESPONSECODE_200_OK;
  }
}
