#include "BaseModel.h"

BaseModel::BaseModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    // 初始化表头
    m_headers = {u8"时间", u8"代码", u8"价格", u8"数量", u8"方向", u8"成交额"};
}

QVariant BaseModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal) {
        return QVariant();
    }
    
    if (section >= 0 && section < m_headers.size()) {
        return m_headers[section];
    }
    
    return QVariant();
}
