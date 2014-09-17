#include <iostream>
#include <julia.h>
#include <sstream>
#include "error.h"
#include "debug.h"

using namespace std;

static string errorMsg(jl_value_t *ex);

static string errorExceptionMsg(jl_value_t *ex)
{
   jl_value_t *f0 = jl_get_nth_field(ex,0);
   stringstream ss;

   ss << jl_iostr_data(f0);
 
   return ss.str();
}

static shared_ptr<nj::Exception> genJuliaErrorException(jl_value_t *ex)
{
   stringstream ss;

   ss << "Julia " << errorExceptionMsg(ex);

   return shared_ptr<nj::Exception>(new nj::JuliaErrorException(ss.str()));
}

static string methodErrorMsg(jl_value_t *ex)
{
   stringstream ss;
   jl_value_t *f0 = jl_get_nth_field(ex,0);
   jl_value_t *f1 = jl_get_nth_field(ex,1);
   size_t numArgs = jl_tuple_len(f1);

   ss << "unmatched method " << jl_gf_name(f0)->name << "(";

   for(size_t i = 0;i < numArgs;i++) 
   {
      jl_value_t *ti = jl_tupleref(f1,i);
      jl_datatype_t *t = (jl_datatype_t*)jl_typeof(ti);

      if(i != 0) ss << ",";
      ss << t->name->name->name;
   }

   ss << ")";

   return ss.str();
}

static shared_ptr<nj::Exception> genJuliaMethodError(jl_value_t *ex)
{
   stringstream ss;

   ss << "Julia " << methodErrorMsg(ex);

   return shared_ptr<nj::Exception>(new nj::JuliaMethodError(ss.str()));
}

static string undefVarErrorMsg(jl_value_t *ex)
{
   jl_value_t *f0 = jl_get_nth_field(ex,0);
   stringstream ss;

   ss << "undefined variable " << ((jl_sym_t*)f0)->name;

   return ss.str();
}

static shared_ptr<nj::Exception> genJuliaUndefVarError(jl_value_t *ex)
{
   stringstream ss;

   ss << "Julia " << undefVarErrorMsg(ex);

   return shared_ptr<nj::Exception>(new nj::JuliaUndefVarError(ss.str()));
}

static string loadErrorMsg(jl_value_t *ex)
{
   jl_value_t *f0 = jl_get_nth_field(ex,0);
   jl_value_t *f1 = jl_get_nth_field(ex,1);
   jl_value_t *f2 = jl_get_nth_field(ex,2);
   stringstream ss;

   ss << "loading " << jl_iostr_data(f0) << " line " << jl_unbox_int64(f1) <<  " " << errorMsg(f2);

   return ss.str();
}

static shared_ptr<nj::Exception> genJuliaLoadError(jl_value_t *ex)
{
   return shared_ptr<nj::Exception>(new nj::JuliaLoadError(loadErrorMsg(ex)));
}

static string errorMsg(jl_value_t *ex)
{
   if(jl_typeis(ex,jl_errorexception_type)) return errorExceptionMsg(ex);
   else if(jl_typeis(ex,jl_methoderror_type)) return methodErrorMsg(ex);
   else if(jl_typeis(ex,jl_undefvarerror_type)) return undefVarErrorMsg(ex);
   else if(jl_typeis(ex,jl_loaderror_type)) return loadErrorMsg(ex);
   else return "unknown error";
}

shared_ptr<nj::Exception> nj::genJuliaError(jl_value_t *ex)
{
   stringstream ss;

   if(jl_typeis(ex,jl_errorexception_type)) return genJuliaErrorException(ex);
   else if(jl_typeis(ex,jl_methoderror_type)) return genJuliaMethodError(ex);
   else if(jl_typeis(ex,jl_undefvarerror_type)) return genJuliaUndefVarError(ex);
   else if(jl_typeis(ex,jl_loaderror_type)) return genJuliaLoadError(ex);
   else
   {
      if(jl_typeis(ex,jl_typeerror_type)) ss << "ex is jl_typeerror_type";
      else ss << "Julia unknown error";
 
      return shared_ptr<Exception>(new InvalidException(ss.str()));
   }
}