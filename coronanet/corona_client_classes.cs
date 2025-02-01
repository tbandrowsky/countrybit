using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Text.Json;

namespace coronanet
{

    public enum FieldTypes
    {
        [DisplayName("none")]
        FtNone = 0,

        [DisplayName("wildcard")]
        FtWildcard = 'w',

        [DisplayName("double")]
        FtDouble = 'd',

        [DisplayName("int64")]
        FtInt64 = 'i',

        [DisplayName("reference")]
        FtReference = 'r',

        [DisplayName("string")]
        FtString = 's',

        [DisplayName("bool")]
        FtBool = 'b',

        [DisplayName("datetime")]
        FtDateTime = 't',

        [DisplayName("object")]
        FtObject = 'o',

        [DisplayName("array")]
        FtArray = 'a',

        [DisplayName("function")]
        FtFunction = 'f',

        [DisplayName("query")]
        FtQuery = 'q',

        [DisplayName("blob")]
        ft_blob = 'x'
	};

    public class ChildObjectBridge
    {
        public string ChildClassName { get; set; } = string.Empty;
        public Dictionary<string, string> CopyValues { get; set; } = new Dictionary<string, string>();
    }

    public class FieldOptions
    {
        ;
    }

    public class ArrayFieldOptions : FieldOptions
    {
        public List<ChildObjectBridge> ChildObjects {  get; set; } = new List<ChildObjectBridge>();
    }

    public class ObjectFieldOptions : FieldOptions
    {
        public string FundamentalType { get; set; } = string.Empty;
        public List<ChildObjectBridge> ChildObjects { get; set; } = new List<ChildObjectBridge>();
    }

    public class StringFieldOptions : FieldOptions
    {
        public int MaximumLength { get; set; };
        public int MinimumLength { get; set; };
        public string MatchPattern { get; set; } = string.Empty;
        List<string> AllowedValues { get; set; } = new List<string>();
    }

    public class IntFieldOptions : FieldOptions
    {
        public int MinValue { get; set; };
        public int MaxValue { get; set; };
    }

    public class DoubleFieldOptions : FieldOptions
    {
        public double MinValue { get; set; };
        public double MaxValue { get; set; };
    }

    public class DateTimeFieldOptions : FieldOptions
    {
        public DateTime MinValue { get; set; };
        public DateTime MaxValue { get; set; };
    }

    public class QueryFromClass
    {
        public string Name { get; set; }
        public string ClassName { get; set; }
        public Dictionary<string, string> Filter { get; set; } = null;
    }

    public class QueryStage
    {
        private string _className;
        public string ClassName => _className;

        public string Name { get; set; };
        public JsonElement StageOutput { get; set; };        
    }

    public class QueryCondition
    {
        private string _className;
        public string ClassName => _className;
    }

    public class QueryContains : QueryCondition 
    {
        QueryContains()
        {
            _className = "contains";
        }
        public string ValuePath { get; set; };
        public string Value { get; set; }
    }

    public class QueryLt : QueryCondition
    {
        QueryLt()
        {
            _className = "lt";
        }
        public string ValuePath { get; set; };
        public string Value { get; set; }
    }

    public class QueryLte : QueryCondition
    {
        QueryLte()
        {
            _className = "lte";
        }
        public string ValuePath { get; set; };
        public string Value { get; set; }
    }

    public class QueryEq : QueryCondition
    {
        QueryEq()
        {
            _className = "eq";
        }
        public string ValuePath { get; set; };
        public string Value { get; set; }
    }

    public class QueryGte : QueryCondition
    {
        QueryGte()
        {
            _className = "gte";
        }
        public string ValuePath { get; set; };
        public string Value { get; set; }
    }

    public class QueryGt : QueryCondition
    {
        QueryGt()
        {
            _className = "gt";
        }
        public string ValuePath { get; set; };
        public string Value { get; set; }
    }

    public class QueryBetween : QueryCondition
    {
        QueryBetween()
        {
            _className = "between";
        }
        public QueryGte Start { get; set; };
        public QueryLt Stop { get; set; }
    }

    public class QueryIn : QueryCondition
    {
        QueryIn()
        {
            _className = "in";
        }
        public string ValuePath { get; set; };
        public string Value { get; set; }
    }

    public class QueryFilter : QueryStage
    {      
        
        public QueryFilter()
        {
            _className = "filter";
        }

        public string Input { get; set; }
        public QueryCondition Condition { get; set; }
    }

    public class QueryJoin : QueryStage
    {
        public QueryJoin()
        {
            _className = "join";
        }

        public string ResultName1 { get; set; };
        public string ResultName2 { get; set; };
        public string Source1 { get; set; };
        public string Source2 { get; set; };
        public string Keys { get; set; }
    }

    public class QueryFieldOptions : FieldOptions
    {
        public List<QueryFromClass> From { get; set; } = new QueryFrom();
        public List<QueryStage> Stages { get; set; } = new QueryStage();
    }

    public class CoronaField
    {


    }

    public class CoronaClass
    {

    }
}
