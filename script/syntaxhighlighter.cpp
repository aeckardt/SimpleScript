#include "syntaxhighlighter.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::blue);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bif\\b" << "\\belse\\b" << "\\bprint\\b";
    for (const QString &pattern : keywordPatterns)
    {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    singleLineCommentFormat.setForeground(Qt::darkGray);
    singleLineComment     = QRegularExpression("#");
    singleLineCommentLine = QRegularExpression("#[^\n]*");

    numberFormat.setForeground(QColor(255, 127, 0));
    rule.pattern = QRegularExpression("\\b[0-9]*\\.?[0-9]*\\b");
    rule.format = numberFormat;
    highlightingRules.append(rule);

    quotationFormat.setForeground(Qt::darkGreen);
    quotation = QRegularExpression("\\\"");
    singleQuote = QRegularExpression("\\\"[^\n]*");
}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(quotation);

    while (startIndex >= 0) {
        QRegularExpressionMatch match = quotation.match(text, startIndex + 1);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1)
        {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
            QRegularExpressionMatch matchSingleQuot = singleQuote.match(text, startIndex);

            endIndex = matchSingleQuot.capturedEnd();
            if (endIndex != -1)
            {
                commentLength = endIndex - startIndex
                                + matchSingleQuot.capturedLength();
                setFormat(startIndex, commentLength, quotationFormat);
            }
        }
        else
        {
            commentLength = endIndex - startIndex
                            + match.capturedLength();
            setFormat(startIndex, commentLength, quotationFormat);
        }
        startIndex = text.indexOf(quotation, startIndex + commentLength);
    }

    QRegularExpressionMatchIterator matchIterator = singleLineComment.globalMatch(text);
    while (matchIterator.hasNext()) {
        QRegularExpressionMatch match = matchIterator.next();
        if (format(match.capturedStart()) != quotationFormat)
        {
            QRegularExpressionMatch matchLine = singleLineCommentLine.match(text, match.capturedStart());
            setFormat(matchLine.capturedStart(), matchLine.capturedLength(), singleLineCommentFormat);
        }
    }
}
