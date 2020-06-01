/*
 * flash.c
 *
 *  Created on: Jun 1, 2020
 *      Author: Administrator
 */

#include "flash.h"



static uint16_t FlashBuffer[STM32FLASH_PAGE_SIZE >> 1];
static uint32_t FLASH_WriteNotCheck(uint32_t Address, const uint16_t *Buffer, uint32_t NumToWrite);

/// 初始化FLASH
void FLASH_Init(void)
{
	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | \
	                       FLASH_FLAG_PGERR | \
	                       FLASH_FLAG_WRPERR);
	HAL_FLASH_Lock();
}

/**
 * 读FLASH
 * @param  Address   要读的起始地址，！！！要求2字节对齐！！！
 * @param  Buffer    存放读取的数据，！！！要求2字节对齐！！！
 * @param  NumToRead 要读取的数据量，单位：半字，！！！要求2字节对齐！！！
 * @return           实际读取到的数据量，单位：字节
 */
uint32_t FLASH_Read(uint32_t Address, uint16_t *Buffer, uint32_t NumToRead)
{
	uint32_t nread = NumToRead;
	uint32_t AddrMax = STM32FLASH_END - 2;

	if (NumToRead == 0 || Buffer == NULL || Address > AddrMax)
		return 0;

	while (Address <= AddrMax && nread)
	{
		*Buffer++ = *(__IO uint16_t *)Address;
		Address += 2;
		nread--;
	}

	return ((NumToRead - nread) << 1);
}

/**
 * 写FLASH
 * @param  Address    写入起始地址，！！！要求2字节对齐！！！
 * @param  Buffer     待写入的数据，！！！要求2字节对齐！！！
 * @param  NumToWrite 要写入的数据量，单位：半字，！！！要求2字节对齐！！！
 * @return            实际写入的数据量，单位：字节
 */
uint32_t FLASH_Write(uint32_t Address, const uint16_t *Buffer, uint32_t NumToWrite)
{
	uint32_t i = 0;
	uint32_t pagepos = 0;         // 页位置
	uint32_t pageoff = 0;         // 页内偏移地址
	uint32_t pagefre = 0;         // 页内空余空间
	uint32_t offset = 0;          // Address在FLASH中的偏移
	uint32_t nwrite = NumToWrite; // 记录剩余要写入的数据量

	/* 非法地址 */
	if (Address < STM32FLASH_BASE || Address > (STM32FLASH_END - 2) || NumToWrite == 0 || Buffer == NULL)
		return 0;

	/* 解锁FLASH */
	HAL_FLASH_Unlock();

	/* 计算偏移地址 */
	offset = Address - STM32FLASH_BASE;

	/* 计算当前页位置 */
	pagepos = offset / STM32FLASH_PAGE_SIZE;

	/* 计算要写数据的起始地址在当前页内的偏移地址 */
	pageoff = ((offset % STM32FLASH_PAGE_SIZE) >> 1);

	/* 计算当前页内空余空间 */
	pagefre = ((STM32FLASH_PAGE_SIZE >> 1) - pageoff);

	/* 要写入的数据量低于当前页空余量 */
	if (nwrite <= pagefre)
		pagefre = nwrite;

	while (nwrite != 0)
	{
		/* 检查是否超页 */
		if (pagepos >= STM32FLASH_PAGE_NUM)
			break;

		/* 读取一页 */
		FLASH_Read(STM32FLASH_BASE + pagepos * STM32FLASH_PAGE_SIZE, FlashBuffer, STM32FLASH_PAGE_SIZE >> 1);

		/* 检查是否需要擦除 */
		for (i = 0; i < pagefre; i++)
		{
			if (*(FlashBuffer + pageoff + i) != 0xFFFF) /* FLASH擦出后默认内容全为0xFF */
				break;
		}

		if (i < pagefre)
		{
			uint32_t count = 0;
			uint32_t index = 0;
			uint32_t PageError = 0;
			FLASH_EraseInitTypeDef pEraseInit;

			/* 擦除一页 */
			pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
			pEraseInit.PageAddress = STM32FLASH_BASE + pagepos * STM32FLASH_PAGE_SIZE;
			pEraseInit.Banks = FLASH_BANK_1;
			pEraseInit.NbPages = 1;
			if (HAL_FLASHEx_Erase(&pEraseInit, &PageError) != HAL_OK)
				break;

			/* 复制到缓存 */
			for (index = 0; index < pagefre; index++)
			{
				*(FlashBuffer + pageoff + index) = *(Buffer + index);
			}

			/* 写回FLASH */
			count = FLASH_WriteNotCheck(STM32FLASH_BASE + pagepos * STM32FLASH_PAGE_SIZE, FlashBuffer, STM32FLASH_PAGE_SIZE >> 1);
			if (count != (STM32FLASH_PAGE_SIZE >> 1))
			{
				nwrite -= count;
				break;
			}
		}
		else
		{
			/* 无需擦除，直接写 */
			uint32_t count = FLASH_WriteNotCheck(Address, Buffer, pagefre);
			if (count != pagefre)
			{
				nwrite -= count;
				break;
			}
		}

		Buffer += pagefre;         /* 读取地址递增         */
		Address += (pagefre << 1); /* 写入地址递增         */
		nwrite -= pagefre;         /* 更新剩余未写入数据量 */

		pagepos++;   /* 下一页               */
		pageoff = 0; /* 页内偏移地址置零     */

		/* 根据剩余量计算下次写入数据量 */
		pagefre = nwrite >= (STM32FLASH_PAGE_SIZE >> 1) ? (STM32FLASH_PAGE_SIZE >> 1) : nwrite;
	}

	/* 加锁FLASH */
	HAL_FLASH_Lock();

	return ((NumToWrite - nwrite) << 1);
}

static uint32_t FLASH_WriteNotCheck(uint32_t Address, const uint16_t *Buffer, uint32_t NumToWrite)
{
	uint32_t nwrite = NumToWrite;
	uint32_t addrmax = STM32FLASH_END - 2;

	while (nwrite)
	{
		if (Address > addrmax)
			break;

		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, Address, *Buffer);
		if ( (*(__IO uint16_t *)Address) != *Buffer )
			break;

		nwrite--;
		Buffer++;
		Address += 2;
	}
	return (NumToWrite - nwrite);
}
