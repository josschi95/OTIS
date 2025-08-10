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
