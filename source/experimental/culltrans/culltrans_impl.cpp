/*___INFO__MARK_BEGIN__*/
/*************************************************************************
 * 
 *  The Contents of this file are made available subject to the terms of
 *  the Sun Industry Standards Source License Version 1.2
 * 
 *  Sun Microsystems Inc., March, 2001
 * 
 * 
 *  Sun Industry Standards Source License Version 1.2
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.2 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
 * 
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 * 
 *   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 * 
 *   Copyright: 2001 by Sun Microsystems, Inc.
 * 
 *   All Rights Reserved.
 * 
 ************************************************************************/
/*___INFO__MARK_END__*/
// culltrans_impl.cpp
// write implementation file

#include <map>
#include <set>
#include <string>
#include <iostream>
#include <fstream>
#include "culltrans_repository.h"
#include "culltrans.h"

#ifdef HAVE_STD
using namespace std;
#endif

// global variables
static string prefix;
static string key;
static int key_type = lEndT;
static string indent("   ");

// write Header
static bool writeHeader(ofstream& impl, map<string, List>::iterator& elem) {
   vector<Elem>::iterator it;
   map<string, List>::iterator list = lists.end();

   impl << "// " << elem->second.name << "_implementation.cpp" << endl;
   impl << "// this file is automatically generated. DO NOT EDIT" << endl << endl;
   // nothing to do from now on if it's not an interface
   if(!elem->second.interf)
      return false;

   impl << "#include <OB/CORBA.h>" << endl;
   impl << "#include \"qidl_common.h\"" << endl;
   impl << "#include \"" << elem->second.name << "_implementation.h\"" << endl;
   for(it=elem->second.elems.begin(); it != elem->second.elems.end(); ++it) {
      if(it->type == lListT && (list = lists.find(it->listType)) != lists.end() && list->second.interf)
         impl << "#include \"" << list->second.name << "_implementation.h\"" << endl;
   }
   impl << "#include \"Master_impl.h\"" << endl;
   impl << "extern \"C\" {" << endl;
   impl << "#include \"sge_api.h\"" << endl;
   impl << "#include \"" << elem->second.file << "\"" << endl;
   impl << "}" << endl;
   impl << endl;

   return true;
}

// get type prefix (e.g. QU_, JB_, CAL_) and key element
bool getType(map<string, List>::iterator& elem) {
   vector<Elem>::iterator it;

   prefix = elem->second.type.substr(0, elem->second.type.find('_')+1);
   for(it=elem->second.elems.begin(); it != elem->second.elems.end(); ++it) 
      if(it->key) {
         key = it->name;
         key_type = it->type;
         break;
      }
   if(key.empty()) {
      cerr << "No key element found for " << elem->second.name << "." << endl;
      return false;
   }

   return true;
}

// write c'tor and d'tor
void writeCtorDtor(ofstream& impl, map<string, List>::iterator& elem) {
   impl << "GE_" << elem->second.name << "_implementation::GE_";
   impl << elem->second.name << "_implementation(const ";
   if(key_type == lUlongT)
      impl << "GE_sge_ulong ";
   else
      impl << "char* ";
   impl << "_key, CORBA_ORB_var o)" << endl;
   //if(key_type == lUlongT)
      impl << indent << ": key(_key), ";
   //else
      //impl << indent << ": key(CORBA_string_dup(_key)), ";
   impl << "self(0), newSelf(0),  orb(o), creation(0), lastEvent(0), ";
   impl << "apiListType(" << elem->second.sge_list_type << ") {}" << endl << endl;
   impl << "GE_" << elem->second.name << "_implementation::GE_";
   impl << elem->second.name << "_implementation(const ";
   if(key_type == lUlongT)
      impl << "GE_sge_ulong ";
   else
      impl << "char* ";
   impl << "_key, const time_t& tm, CORBA_ORB_var o)" << endl;
   //if(key_type == lUlongT)
      impl << indent << ": key(_key), ";
   //else
      //impl << indent << ": key(CORBA_string_dup(_key)), ";
   impl << "self(0), newSelf(0),  orb(o), creation(tm), lastEvent(0) {}" << endl << endl;
   impl << "GE_" << elem->second.name << "_implementation::~GE_";
   impl << elem->second.name << "_implementation() {}" << endl << endl;
}

// write get functions
bool writeGetFuncs(ofstream& impl, map<string, List>::iterator& elem) {
   vector<Elem>::iterator it;
   map<string, List>::iterator list = lists.end();

   for(it=elem->second.elems.begin(); it != elem->second.elems.end(); ++it) {
      if(it->idlonly || it->key || ((it->type == lListT) && ((list = lists.find(it->listType)) != lists.end()) && list->second.interf))
         continue;

      if(it->type == lListT) {
         list = lists.find(it->listType);
         if(list != lists.end())
            if(it->object)
               impl << "GE_" << list->second.name << "* ";
            else
               impl << "GE_" << list->second.name << "Seq* ";
         else {
            cerr << "Could not find " << it->listType << "." << endl;
            return false;
         }
      }
      else
         impl << multiType2sgeType[it->type] << ' ';

      impl << "GE_" << elem->second.name << "_implementation::get_" << it->name;
      impl << "(CORBA_Context* ctx) {" << endl;
      impl << indent << "QENTER(\"" << elem->second.name;
      impl << "::get_" << it->name << "\");" << endl;
      impl << indent << "AUTO_LOCK_MASTER;" << endl;
      impl << indent << "qidl_authenticate(ctx);" << endl;
      impl << indent << "getSelf();" << endl << endl;
      if(it->type == lListT) {
         list = lists.find(it->listType);
         if(!list->second.interf) {
            impl << indent << "lList* lp = lGetList(self, ";
            impl << prefix << it->name << ");" << endl;
            impl << indent << "GE_" << list->second.name << "Seq* foo = ";
            impl << "cull2" << list->second.name << "Seq(lp);" << endl;
         }
         else {
            // get other's type prefix (e.g. QU_, JB_, CAL_) and key element
            string other_prefix = list->second.type.substr(0, list->second.type.find('_')+1);
            vector<Elem>::iterator other;
            for(other=list->second.elems.begin(); other != list->second.elems.end(); ++other) 
               if(other->key)
                  break;
            if(other == list->second.elems.end()) {
               cerr << "No key element found for " << list->second.name << "." << endl;
               return false;
            }
            impl << indent << "// build the list of object refs" << endl;
            impl << indent << "lList* lp = lGetList(self, ";
            impl << prefix << it->name << ");" << endl;
            impl << indent << "lListElem* lep;" << endl;
            impl << indent << "GE_" << list->second.name << "_implementation* x;" << endl << endl;
            impl << indent << "GE_" << list->second.name << "Seq* foo =";
            impl << " new GE_" << list->second.name << "Seq();" << endl;
            impl << indent << "for_each_cpp(lep, lp) {" << endl;
            impl << indent << indent << "x = GE_Master_impl::instance()";
            impl << "->get" << list->second.name << "Impl(lGet";
            impl << multiType2getType[other->type] << "(lep, ";
            impl << other_prefix << other->name << "));" << endl;
            impl << endl << indent << indent << "if(x)" << endl;
            impl << indent << indent << indent << "foo->append(GE_";
            impl << list->second.name << "_implementation::_duplicate(x));" << endl;
            impl << indent << "}" << endl << endl;

         }
      }
      else if(it->type == lStringT) {
         impl << indent << "char* temp = lGetString(self, ";
         impl << prefix << it->name << ");" << endl;
         impl << indent << "GE_sge_string foo = CORBA_string_dup(temp?temp:\"\");" << endl;
      }
      else {
         impl << indent << multiType2sgeType[it->type] << " foo = lGet";
         impl << multiType2getType[it->type] << "(self, ";
         impl << prefix << it->name << ");" << endl;   
      }
      impl << indent << "return foo;" << endl;
      impl << "}" << endl << endl;
   }

   return true;
}
 
// write set functions
bool writeSetFuncs(ofstream& impl, map<string, List>::iterator& elem) {
   vector<Elem>::iterator it;
   map<string, List>::iterator list = lists.end();

   for(it=elem->second.elems.begin(); it != elem->second.elems.end(); ++it) {
      if(it->idlonly || it->readonly || it->key || ((it->type == lListT) && ((list = lists.find(it->listType)) != lists.end()) && list->second.interf))
         continue;

      impl << "GE_sge_ulong GE_" << elem->second.name << "_implementation::set_" << it->name << "(";
      if(it->type == lListT) {
         list = lists.find(it->listType);
         if(list != lists.end()) {
            impl << "const GE_" << list->second.name << "Seq& ";
         }
         else {
            cerr << "Could not find " << it->listType << "." << endl;
            return false;
         }
      }
      else if(it->type == lStringT)
         impl << "const char* ";
      else
         impl << multiType2sgeType[it->type] << ' ';
      impl << "val, CORBA_Context* ctx) {" << endl;
      impl << indent << "QENTER(\"" << elem->second.name;
      impl << "::set_" << it->name << "\");" << endl;
      impl << indent << "AUTO_LOCK_MASTER;" << endl;
      impl << indent << "qidl_authenticate(ctx);" << endl;
      impl << indent << "getSelf();" << endl << endl;
      impl << indent << "if(creation != 0) {" << endl;
      impl << indent << indent << "lSet" << multiType2getType[it->type];
      impl << "(self, " << prefix << it->name << ", ";
      if(it->type == lListT) {
         impl << list->second.name << "Seq2cull((GE_" << list->second.name;
         impl << "Seq&)val));" << endl;
      }
      else if(it->type == lStringT)
         impl << "(char*)val);" << endl;
      else
         impl << "val);" << endl;
      impl << indent << indent << "return 0;" << endl;
      impl << indent << "}" << endl << endl;
      impl << indent << "lListPtr      lp;" << endl;
      impl << indent << "lListPtr     alp;" << endl;
      impl << indent << "lListElem*   lep;" << endl;
      impl << indent << "lEnumeration* what;" << endl << endl;
      impl << indent << "lp = lCreateList(\"My " << it->name << " list\", ";
      impl << prefix << "Type);" << endl;
      impl << indent << "lAppendElem(lp, (lep = lCopyElem(self)));" << endl;
      impl << indent << "lSet" << multiType2getType[it->type] << "(lep, ";
      impl << prefix << it->name << ", ";
      if(it->type == lListT) {
         impl << list->second.name << "Seq2cull((GE_" << list->second.name;
         impl << "Seq&)val));" << endl;
      }
      else if(it->type == lStringT)
         impl << "(char*)val);" << endl;
      else
         impl << "val);" << endl;
      impl << indent << "what = lWhat(\"%T(%I %I)\", " << prefix << "Type, ";
      impl << prefix << key << ", " << prefix << it->name << ");" << endl;
      impl << indent << "alp = sge_api(apiListType";
      impl << ", SGE_API_MOD, &lp, NULL, what);" << endl;
      impl << indent << "lFreeWhat(&what);" << endl;
      impl << indent << "throwErrors(alp);" << endl;
      impl << indent << "return lastEvent;" << endl;
      impl << "}" << endl << endl;
   }

   return true;
}

// write get content
bool writeGetContent(ofstream& impl, map<string, List>::iterator& elem) {
   vector<Elem>::iterator it;
   map<string, List>::iterator list = lists.end();

   impl << "GE_contentSeq* GE_" << elem->second.name << "_implementation::get_content(CORBA_Context* ctx) {" << endl;
   impl << indent << "QENTER(\"" << elem->second.name;
   impl << "::get_content\");" << endl;
   impl << indent << "AUTO_LOCK_MASTER;" << endl;
   impl << indent << "qidl_authenticate(ctx);" << endl;
   impl << indent << "getSelf();" << endl << endl;
   
   impl << indent << "// build content sequence" << endl;
   impl << indent << "GE_contentSeq* cs = new GE_contentSeq;" << endl;
   impl << indent << "char* dummy;" << endl << endl;
   impl << indent << "cs->length(" << elem->second.elems.size() << ");" << endl << endl;

   unsigned long i=0;
   for(it=elem->second.elems.begin(), i=0; it != elem->second.elems.end(); ++it, i++) {
      impl << indent << "// " << it->name << endl;
      impl << indent << "cs->operator[](" << i << ").elem = GE_" << prefix << it->name << ";" << endl;
   	impl << indent << "cs->operator[](" << i << ").value <<= ";
      if(it->type == lListT) {
         list = lists.find(it->listType);
         if(list != lists.end())
            impl << "get_" << it->name << "(GE_Master_impl::instance()->getMasterContext());" << endl;
         else {
            cerr << "Could not find " << it->listType << "." << endl;
            return false;
         }
      }
      else if(it->idlonly)
         impl << "get_" << it->name << "(GE_Master_impl::instance()->getMasterContext());" << endl;
      else if(it->type == lBoolT)
         impl << "CORBA_Any::from_boolean(" << "(" << multiType2sgeType[it->type] << ")lGet" << multiType2getType[it->type] << "(self, " << prefix << it->name << "));" << endl;
      else if(it->type == lStringT)
         impl << "(dummy = lGetString(self, " << prefix << it->name << "))?dummy:\"\";" << endl;
      else
         impl << "(" << multiType2sgeType[it->type] << ")lGet" << multiType2getType[it->type] << "(self, " << prefix << it->name << ");" << endl;
   }
   impl << indent << "return cs;" << endl;
   impl << "}" << endl << endl;
   
   return true;
}

// write set content
bool writeSetContent(ofstream& impl, map<string, List>::iterator& elem) {
   vector<Elem>::iterator it;
   map<string, List>::iterator list = lists.end();

   impl << "GE_sge_ulong GE_" << elem->second.name << "_implementation::set_content(const GE_contentSeq& cs, CORBA_Context* ctx) {" << endl;
   impl << indent << "QENTER(\"" << elem->second.name;
   impl << "::set_content\");" << endl;
   impl << indent << "AUTO_LOCK_MASTER;" << endl;
   impl << indent << "qidl_authenticate(ctx);" << endl;
   impl << indent << "getSelf();" << endl << endl;
   impl << indent << "// intermediate variables" << endl;
   for(list=lists.begin(); list != lists.end(); ++list)
      for(it=elem->second.elems.begin(); it != elem->second.elems.end(); ++it)
         if(it->type == lListT && it->listType == list->second.type) {
            impl << indent << "GE_" << list->second.name;
            if(!it->object)
               impl << "Seq";
            impl << "* " << list->second.name << "_val;" << endl;
            break;
         }
   for(int x=1; x<lListT; x++)
      impl << indent << multiType2sgeType[x] << " " << multiType2getType[x] << "_val;" << endl;
   impl << indent << "lList* " << multiType2getType[lListT] << "_val;" << endl;
   impl << indent << "int* nameVector = new int[cs.length()+1];" << endl;
   impl << indent << "lListPtr lp = lCreateList(\"set content\", " << elem->second.type << ");" << endl;
   impl << indent << "lListElem* lep = lCreateElem(" << elem->second.type << ");" << endl;
   impl << indent << "lAppendElem(lp, lep);" << endl;
   impl << indent << "// now pretend we're CORBA-only" << endl;
   impl << indent << "time_t old_creation = creation;" << endl;
   impl << indent << "creation = 1;" << endl;
   impl << indent << "lListElem* oldSelf = self;" << endl;
   impl << indent << "self = lep;" << endl << endl;

   impl << indent << "// check the sequence" << endl;
   impl << indent << "for(CORBA_ULong i=0; i<cs.length(); i++) {" << endl;
   impl << indent << indent << "switch(cs[i].elem) {" << endl;
   for(it=elem->second.elems.begin(); it != elem->second.elems.end(); ++it) {
      // don't try to set readonly elems
      // maybe throwing an exception would be better
      // but for now these are handled in default:
      if(it->readonly)
         continue;
      impl << indent << indent << "case " << prefix << it->name << ":" << endl;
      if(it->type == lListT) {
         list = lists.find(it->listType);
         impl << indent << indent << indent << "cs[i].value >>= " << list->second.name << "_val;" << endl;
         if(!list->second.interf)
            impl << indent << indent << indent << multiType2getType[it->type] << "_val = " << list->second.name << "Seq2cull(*" << list->second.name << "_val);" << endl;
         else {
            impl << indent << indent << indent << "set_" << it->name << "(";
            if(!it->object)
               impl << "*";
            impl << list->second.name << "_val, GE_Master_impl::instance()->getMasterContext());" << endl;
         }
      }
      else
         impl << indent << indent << indent << "cs[i].value >>= " << multiType2getType[it->type] << "_val;" << endl;
      if(it->type != lListT || (it->type == lListT && !list->second.interf))
         impl << indent << indent << indent << "lSet" << multiType2getType[it->type] << "(lep, " << prefix << it->name << ", " << multiType2getType[it->type] << "_val);" << endl;
      else if(it->type != lListT && it->idlonly)
         impl << indent << indent << indent << "set_" << it->name << "(" << multiType2getType[it->type] << "_val , GE_Master_impl::instance()->getMasterContext());" << endl;
      impl << indent << indent << indent << "break;" << endl;
   }
   impl << indent << indent << "default:" << endl;
   impl << indent << indent << indent << "break;" << endl;
   impl << indent << indent << "}" << endl;
   impl << indent << indent << "nameVector[i] = cs[i].elem;" << endl;
   impl << indent << "}" << endl;
   impl << indent << "nameVector[cs.length()] = NoName;" << endl << endl;
   impl << indent << "// don't forget to set back" << endl;
   impl << indent << "creation = old_creation;" << endl;
   impl << indent << "self = oldSelf;" << endl << endl;
   impl << indent << "// now set the state" << endl;
   impl << indent << "if(creation) {" << endl;
   impl << indent << indent << "lFreeElem(&self);" << endl;
   impl << indent << indent << "self = lCopyElem(lep);" << endl;
   impl << indent << "}" << endl;
   impl << indent << "else {" << endl;
   impl << indent << indent << "lListPtr alp;" << endl;
   impl << indent << indent << "lEnumeration* what = lIntVector2What(" << elem->second.type << ", nameVector);" << endl;
   impl << indent << indent << "alp = sge_api(apiListType, SGE_API_MOD, &lp, NULL, what);" << endl;
   impl << indent << indent << "lFreeWhat(&what);" << endl;
   impl << indent << indent << "throwErrors(alp);" << endl;
   impl << indent << "}" << endl << endl;
   impl << indent << "return lastEvent;" << endl;
   impl << "}" << endl << endl;

   return true;
}

// writeImplementation
// writes out the implementation file for a given interface or struct
// this file contains only those functions that can be generated
// automatically by the meta data
bool writeImplementation(map<string, List>::iterator& elem) {
   cout << "Creating implementation file for " << elem->second.name << endl;
   
   vector<Elem>::iterator it;
   map<string, List>::iterator list = lists.end();
   
   // open output file
   string file(elem->second.name);
   file += "_implementation.cpp";
   ofstream impl(file.c_str());
   if(!impl) {
      cerr << "Could not open output file for " << elem->second.name << endl;
      return false;
   }

   // write header and #includes
   // returns false if not an interface
   if(!writeHeader(impl, elem))
      return true;

   // get type prefix (e.g. QU_, JB_, CAL_) and key element
   if(!getType(elem))
      return false;

   // write c'tor and d'tor
   writeCtorDtor(impl, elem);

   // write get functions
   writeGetFuncs(impl, elem);
  
   // write set functions
   writeSetFuncs(impl, elem);
  
   // write get content
   writeGetContent(impl, elem);

   // write set content
   writeSetContent(impl, elem);

   impl.close();
   return true;   
}