#pragma once

enum class Severity {
    Emergency,
    Alert,
    Critical,
    Error,
    Warning,
    Notice,
    Informational,
    Debug,
};

enum class ComparisonOperator {
    eq,  // ==
    ne,  // !=
    lt,  // <
    lte, // <=
    gt,  // >
    gte,  // >=
};

enum class StringComparison {
    ExactMatch, // == comparison
    Contains, // QString.contains()
    StartsWith, // QString.startsWith
    // Regex?
};



/*const QMap<ComparisonOperator, QString>& compOperatorToStringMap() {
    static const QMap<ComparisonOperator, QString> map = {
        { ComparisonOperator::eq, "==" },
        { ComparisonOperator::ne, "!=" },
        { ComparisonOperator::lt, "<" },
        { ComparisonOperator::lte, "<=" },
        { ComparisonOperator::gt, ">" },
        { ComparisonOperator::gte, ">=" },
    };
    return map;
}

const QMap<QString, ComparisonOperator>& stringToCompOperatorMap() {
    static const QMap<QString, ComparisonOperator> map = {
        { "==", ComparisonOperator::eq  },
        { "!=", ComparisonOperator::ne  },
        { "<" , ComparisonOperator::lt  },
        { "<=", ComparisonOperator::lte },
        { ">" , ComparisonOperator::gt  },
        { ">=", ComparisonOperator::gte },
    };
    return map;
}*/
