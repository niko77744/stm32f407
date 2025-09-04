#include "lfs_config.h"

#define W25Qxx_SECTOR_SIZE 4096
#define W25Qxx_SECTOR_NUM 256
int lfs_spi_flash_init(struct lfs_config *cfg);
int lfs_spi_flash_read(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size);
int lfs_spi_flash_prog(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
int lfs_spi_flash_erase(const struct lfs_config *cfg, lfs_block_t block);
int lfs_spi_flash_sync(const struct lfs_config *cfg);

int lfs_spi_flash_init(struct lfs_config *cfg)
{
    cfg->read = lfs_spi_flash_read;
    cfg->prog = lfs_spi_flash_prog;
    cfg->erase = lfs_spi_flash_erase;
    cfg->sync = lfs_spi_flash_sync;

    // ��С��ȡ�ֽ��������еĶ�ȡ�����ֽ�������������������
    cfg->read_size = 16;
    // ��Сд���ֽ��������е�д������ֽ�������������������
    cfg->prog_size = 16;
    // ������������ֽ�������ѡ�Ӱ�� RAM ���ģ����Ա���������ߴ��
    // ����ÿ���ļ�����ռ��һ���飬�����Ƕ�ȡ��д������ֽ�����������
    cfg->block_size = W25Qxx_SECTOR_SIZE;
    // �豸�Ͽɲ������������������
    cfg->block_count = W25Qxx_SECTOR_NUM;
    // littlefs ϵͳɾ��Ԫ������־����Ԫ�����ƶ�����һ����֮ǰ�Ĳ�����������
    // ����ȡֵ��ΧΪ 100 ~ 1000���ϴ���ֵ�нϺõ����ܵ��ǻᵼ��ĥ��ֲ���һ��
    // ȡֵ -1 �Ļ�����Ϊ���ÿ鼶ĥ�����
    cfg->block_cycles = 500;
    // �黺���С��ÿ�����涼���� RAM �л���һ���ֿ����ݣ�
    // littlefs ϵͳ��Ҫһ����ȡ���桢һ��д�뻺�棬ÿ���ļ�����Ҫһ������Ļ��档
    // ����Ļ������ͨ���洢��������ݲ����ʹ��̷����������ֶ����������
    cfg->cache_size = 16;
    // ���л����С����������л��������߷�������пɱ����ֵĿ�����
    // �������ʱÿ�β������ٸ��飬16�ͱ�ʾÿ�η���16����
    // ���л����Խ��յ�bitλ��ʽ���洢���� RAM �е�һ���ֽڿ��Զ�Ӧ8����
    // ��ֵ������8��������
    cfg->lookahead_size = 16;
    return LFS_ERR_OK;
}

/*
 * @brief ��ָ�����ڵ�ĳ���������
 * @param [in] lfs_config��ʽ����
 * @param [in] block �߼��������ţ���0��ʼ
 * @param [in] off ����ƫ�ƣ���ֵ���ܱ�read_size����
 * @param [out] �������ݵ����������
 * @param [in] size Ҫ��ȡ���ֽ�������ֵ���ܱ�read_size������lfs�ڶ�ȡʱ��ȷ�������飻
 * @retval 0 �ɹ�, < 0 ������
 */
int lfs_spi_flash_read(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
    // check if read is valid
    // LFS_ASSERT(off % cfg->read_size == 0);
    // LFS_ASSERT(size % cfg->read_size == 0);
    // LFS_ASSERT(block < cfg->block_count);
    uint32_t addr = block * cfg->block_size + off;
    const sfud_flash *flash = sfud_get_device(0u);
    sfud_read(flash, addr, size, (uint8_t *)buffer);
    return LFS_ERR_OK;
}

/*
 * @brief ������д��ָ�����ڵ�ĳ���򡣸���������Ѿ��ȱ������������Է��� LFS_ERR_CORRUPT ��ʾ�ÿ�����
 * @param [in] lfs_config��ʽ����
 * @param [in] block �߼��������ţ���0��ʼ
 * @param [in] off ����ƫ�ƣ���ֵ���ܱ�rprog_size����
 * @param [in] д�����ݵĻ�����
 * @param [in] size Ҫд����ֽ�������ֵ���ܱ�read_size������lfs�ڶ�ȡʱ��ȷ�������飻
 * @retval 0 �ɹ�, < 0 ������
 */
int lfs_spi_flash_prog(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
    // check if write is valid
    // LFS_ASSERT(off % cfg->prog_size == 0);
    // LFS_ASSERT(size % cfg->prog_size == 0);
    // LFS_ASSERT(block < cfg->block_count);

    uint32_t addr = block * cfg->block_size + off;
    const sfud_flash *flash = sfud_get_device(0u);
    sfud_write(flash, addr, size, (uint8_t *)buffer);
    return LFS_ERR_OK;
}

/*
 * @brief ����ָ���顣����д��֮ǰ�����ȱ������������������״̬��δ����
 * @param [in] lfs_config��ʽ����
 * @param [in] block Ҫ�������߼��������ţ���0��ʼ
 * @retval 0 �ɹ�, < 0 ������
 */
int lfs_spi_flash_erase(const struct lfs_config *cfg, lfs_block_t block)
{
    // check if erase is valid
    LFS_ASSERT(block < cfg->block_count);
    const sfud_flash *flash = sfud_get_device(0u);
    sfud_erase(flash, block * cfg->block_size, cfg->block_size);
    return LFS_ERR_OK;
}

/*
 * @brief �Եײ���豸��ͬ�����������ײ���豸��û��ͬ�������������ֱ�ӷ���
 * @param [in] lfs_config��ʽ����;
 * @retval 0 �ɹ�, < 0 ������
 */
int lfs_spi_flash_sync(const struct lfs_config *cfg)
{
    return LFS_ERR_OK;
}
