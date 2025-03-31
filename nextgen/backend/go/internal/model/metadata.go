package model

type MetaDataKey string
type MetaDataValue any
type MetaDataType string

const (
	MetaTypeOrdinal  = "ordinal"
	MetaTypeCategory = "category"
)

type MetaDataDict map[MetaDataKey]MetaDataType

type MetaDataRepo map[MetaDataKey]MetaDataValue
