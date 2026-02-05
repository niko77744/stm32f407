#include <smf.h>
#include <stdio.h>

/*
 * Flat Test Transition:
 *
 * A_ENTRY --> A_RUN --> A_EXIT --> B_ENTRY --> B_RUN --|
 *                                                      |
 * |----------------------------------------------------|
 * |
 * |--> B_EXIT --> C_ENTRY --> C_RUN --> C_EXIT --> D_ENTRY
 *
 */

#define TEST_OBJECT(o) ((struct test_object *)o)

#define SMF_RUN 3

#define STATE_A_ENTRY_BIT BIT(0)
#define STATE_A_RUN_BIT BIT(1)
#define STATE_A_EXIT_BIT BIT(2)

#define STATE_B_ENTRY_BIT BIT(3)
#define STATE_B_RUN_BIT BIT(4)
#define STATE_B_EXIT_BIT BIT(5)

#define STATE_C_ENTRY_BIT BIT(6)
#define STATE_C_RUN_BIT BIT(7)
#define STATE_C_EXIT_BIT BIT(8)

#define TEST_ENTRY_VALUE_NUM 0
#define TEST_RUN_VALUE_NUM 4
#define TEST_EXIT_VALUE_NUM 8
#define TEST_VALUE_NUM 9

static uint32_t test_value[] = {
    0x00,  /* STATE_A_ENTRY */
    0x01,  /* STATE_A_RUN */
    0x03,  /* STATE_A_EXIT */
    0x07,  /* STATE_B_ENTRY */
    0x0f,  /* STATE_B_RUN */
    0x1f,  /* STATE_B_EXIT */
    0x3f,  /* STATE_C_ENTRY */
    0x7f,  /* STATE_C_RUN */
    0xff,  /* STATE_C_EXIT */
    0x1ff, /* FINAL VALUE */
};

/* Forward declaration of test_states */
static const struct smf_state test_states[];

/* List of all TypeC-level states */
enum test_state
{
    STATE_A,
    STATE_B,
    STATE_C,
    STATE_D
};

enum terminate_action
{
    NONE,
    ENTRY,
    RUN,
    EXIT
};

static struct test_object
{
    struct smf_ctx ctx;
    uint32_t transition_bits;
    uint32_t tv_idx;
    enum terminate_action terminate;
} test_obj;

static void state_a_entry(void *obj)
{
    /**
        1. smf_get_current_executing_state()
        返回当前正在执行的状态
        在平面状态机中，就是当前状态
        在层次状态机中，可能是父状态或子状态，取决于当前执行点

        2. smf_get_current_leaf_state()
        返回当前叶子状态（层次状态机中最深层的状态）
        在平面状态机中，与当前状态相同
        在层次状态机中，总是返回最底层的状态
     */
    if (smf_get_current_executing_state(SMF_CTX(obj)) != &test_states[STATE_A])
    {
        printf("扁平测试转换 在入口处无法获取当前正在执行的状态 预期状态 A 状态\n");
    }
    if (smf_get_current_leaf_state(SMF_CTX(obj)) != &test_states[STATE_A])
    {
        printf("在入口处无法获取当前的叶子状态 预期状态 A 状态\n");
    }

    // zassert_equal(smf_get_current_executing_state(SMF_CTX(obj)), &test_states[STATE_A],
    //               "Fail to get the currently-executing state at entry. Expected: State A");
    // zassert_equal(smf_get_current_leaf_state(SMF_CTX(obj)), &test_states[STATE_A],
    //               "Fail to get the current leaf state at entry. Expected: State A");

    struct test_object *o = TEST_OBJECT(obj);

    o->tv_idx = 0;
    if (o->transition_bits != test_value[o->tv_idx])
    {
        printf("测试状态 A 进入失败\n");
    }

    // zassert_equal(o->transition_bits, test_value[o->tv_idx], "Test State A entry failed");

    if (o->terminate == ENTRY)
    {
        smf_set_terminate(obj, -1);
        return;
    }

    o->transition_bits |= STATE_A_ENTRY_BIT;
}

static enum smf_state_result state_a_run(void *obj)
{
    if (smf_get_current_executing_state(SMF_CTX(obj)) != &test_states[STATE_A])
    {
        printf("在运行过程中未能获取当前执行状态。预期状态为：状态 A\n");
    }
    if (smf_get_current_leaf_state(SMF_CTX(obj)) != &test_states[STATE_A])
    {
        printf("在运行过程中未能获取当前叶节点的状态。预期状态为：状态 A\n");
    }

    // zassert_equal(smf_get_current_executing_state(SMF_CTX(obj)), &test_states[STATE_A],"Fail to get the currently-executing state at run. Expected: State A");
    // zassert_equal(smf_get_current_leaf_state(SMF_CTX(obj)), &test_states[STATE_A], "Fail to get the current leaf state at run. Expected: State A");

    struct test_object *o = TEST_OBJECT(obj);

    o->tv_idx++;
    if (o->transition_bits != test_value[o->tv_idx])
    {
        printf("测试状态 A 的运行失败了\n");
    }
    // zassert_equal(o->transition_bits, test_value[o->tv_idx], "Test State A run failed");

    o->transition_bits |= STATE_A_RUN_BIT;

    smf_set_state(SMF_CTX(obj), &test_states[STATE_B]);
    return SMF_EVENT_HANDLED;
}

static void state_a_exit(void *obj)
{
    if (smf_get_current_executing_state(SMF_CTX(obj)) != &test_states[STATE_A])
    {
        printf("在退出时未能获取当前执行状态。预期状态为：状态 A\n");
    }
    if (smf_get_current_leaf_state(SMF_CTX(obj)) != &test_states[STATE_A])
    {
        printf("在退出时未能获取当前叶节点的状态。预期状态为：状态 A\n");
    }

    // zassert_equal(smf_get_current_executing_state(SMF_CTX(obj)), &test_states[STATE_A], "Fail to get the currently-executing state at exit. Expected: State A");
    // zassert_equal(smf_get_current_leaf_state(SMF_CTX(obj)), &test_states[STATE_A],"Fail to get the current leaf state at exit. Expected: State A");

    struct test_object *o = TEST_OBJECT(obj);

    o->tv_idx++;
    if (o->transition_bits != test_value[o->tv_idx])
    {
        printf("测试状态 A 退出失败\n");
    }
    // zassert_equal(o->transition_bits, test_value[o->tv_idx], "Test State A exit failed");

    o->transition_bits |= STATE_A_EXIT_BIT;
}

static void state_b_entry(void *obj)
{
    if (smf_get_current_executing_state(SMF_CTX(obj)) != &test_states[STATE_B])
    {
        printf("在进入时未能获取当前执行状态。预期状态为：状态 B\n");
    }
    if (smf_get_current_leaf_state(SMF_CTX(obj)) != &test_states[STATE_B])
    {
        printf("在进入时未能获取当前叶节点状态。预期状态为：状态 B\n");
    }

    // zassert_equal(smf_get_current_executing_state(SMF_CTX(obj)), &test_states[STATE_B], "Fail to get the currently-executing state at entry. Expected: State B");
    // zassert_equal(smf_get_current_leaf_state(SMF_CTX(obj)), &test_states[STATE_B], "Fail to get the current leaf state at entry. Expected: State B");

    struct test_object *o = TEST_OBJECT(obj);

    o->tv_idx++;
    if (o->transition_bits != test_value[o->tv_idx])
    {
        printf("测试状态 B 的入口失败了\n");
    }
    // zassert_equal(o->transition_bits, test_value[o->tv_idx], "Test State B entry failed");

    o->transition_bits |= STATE_B_ENTRY_BIT;
}

static enum smf_state_result state_b_run(void *obj)
{
    if (smf_get_current_executing_state(SMF_CTX(obj)) != &test_states[STATE_B])
    {
        printf("在运行过程中未能获取当前执行状态。预期状态为：状态 B\n");
    }
    if (smf_get_current_leaf_state(SMF_CTX(obj)) != &test_states[STATE_B])
    {
        printf("在运行过程中未能获取当前叶节点的状态。预期状态为：状态 B\n");
    }
    // zassert_equal(smf_get_current_executing_state(SMF_CTX(obj)), &test_states[STATE_B], "Fail to get the currently-executing state at run. Expected: State B");
    // zassert_equal(smf_get_current_leaf_state(SMF_CTX(obj)), &test_states[STATE_B], "Fail to get the current leaf state at run. Expected: State B");

    struct test_object *o = TEST_OBJECT(obj);

    o->tv_idx++;
    if (o->transition_bits != test_value[o->tv_idx])
    {
        printf("测试状态 B 运行失败\n");
    }
    // zassert_equal(o->transition_bits, test_value[o->tv_idx], "Test State B run failed");

    if (o->terminate == RUN)
    {
        smf_set_terminate(obj, -1);
        return SMF_EVENT_HANDLED;
    }

    o->transition_bits |= STATE_B_RUN_BIT;

    smf_set_state(SMF_CTX(obj), &test_states[STATE_C]);
    return SMF_EVENT_HANDLED;
}

static void state_b_exit(void *obj)
{
    if (smf_get_current_executing_state(SMF_CTX(obj)) != &test_states[STATE_B])
    {
        printf("在退出时未能获取当前执行状态。预期状态为：状态 B\n");
    }
    if (smf_get_current_leaf_state(SMF_CTX(obj)) != &test_states[STATE_B])
    {
        printf("在退出时未能获取当前叶节点状态。预期状态：状态 B\n");
    }
    // zassert_equal(smf_get_current_executing_state(SMF_CTX(obj)), &test_states[STATE_B], "Fail to get the currently-executing state at exit. Expected: State B");
    // zassert_equal(smf_get_current_leaf_state(SMF_CTX(obj)), &test_states[STATE_B], "Fail to get the current leaf state at exit. Expected: State B");

    struct test_object *o = TEST_OBJECT(obj);

    o->tv_idx++;
    if (o->transition_bits != test_value[o->tv_idx])
    {
        printf("测试状态 B 退出失败\n");
    }
    // zassert_equal(o->transition_bits, test_value[o->tv_idx], "Test State B exit failed");
    o->transition_bits |= STATE_B_EXIT_BIT;
}

static void state_c_entry(void *obj)
{
    if (smf_get_current_executing_state(SMF_CTX(obj)) != &test_states[STATE_C])
    {
        printf("在进入该程序时未能获取到当前执行状态。预期状态为：状态 C\n");
    }
    if (smf_get_current_leaf_state(SMF_CTX(obj)) != &test_states[STATE_C])
    {
        printf("在进入时未能获取当前叶子节点的状态。预期状态为：状态 C\n");
    }
    // zassert_equal(smf_get_current_executing_state(SMF_CTX(obj)), &test_states[STATE_C], "Fail to get the currently-executing state at entry. Expected: State C");
    // zassert_equal(smf_get_current_leaf_state(SMF_CTX(obj)), &test_states[STATE_C], "Fail to get the current leaf state at entry. Expected: State C");

    struct test_object *o = TEST_OBJECT(obj);

    o->tv_idx++;
    if (o->transition_bits != test_value[o->tv_idx])
    {
        printf("测试状态 C 的输入失败了\n");
    }
    // zassert_equal(o->transition_bits, test_value[o->tv_idx], "Test State C entry failed");
    o->transition_bits |= STATE_C_ENTRY_BIT;
}

static enum smf_state_result state_c_run(void *obj)
{
    if (smf_get_current_executing_state(SMF_CTX(obj)) != &test_states[STATE_C])
    {
        printf("在运行过程中未能获取当前执行状态。预期状态为：状态 C\n");
    }
    if (smf_get_current_leaf_state(SMF_CTX(obj)) != &test_states[STATE_C])
    {
        printf("在运行过程中未能获取当前叶节点的状态。预期状态为：状态 C\n");
    }
    // zassert_equal(smf_get_current_executing_state(SMF_CTX(obj)), &test_states[STATE_C],  "Fail to get the currently-executing state at run. Expected: State C");
    // zassert_equal(smf_get_current_leaf_state(SMF_CTX(obj)), &test_states[STATE_C], "Fail to get the current leaf state at run. Expected: State C");

    struct test_object *o = TEST_OBJECT(obj);

    o->tv_idx++;
    if (o->transition_bits != test_value[o->tv_idx])
    {
        printf("测试状态 C 运行失败\n");
    }
    // zassert_equal(o->transition_bits, test_value[o->tv_idx], "Test State C run failed");
    o->transition_bits |= STATE_C_RUN_BIT;

    smf_set_state(SMF_CTX(obj), &test_states[STATE_D]);
    return SMF_EVENT_HANDLED;
}

static void state_c_exit(void *obj)
{
    if (smf_get_current_executing_state(SMF_CTX(obj)) != &test_states[STATE_C])
    {
        printf("在退出时未能获取当前执行状态。预期状态 C\n");
    }
    if (smf_get_current_leaf_state(SMF_CTX(obj)) != &test_states[STATE_C])
    {
        printf("在退出时未能获取当前叶节点状态。预期状态为：状态 C\n");
    }
    // zassert_equal(smf_get_current_executing_state(SMF_CTX(obj)), &test_states[STATE_C],
    //               "Fail to get the currently-executing state at exit. Expected: State C");
    // zassert_equal(smf_get_current_leaf_state(SMF_CTX(obj)), &test_states[STATE_C],
    //               "Fail to get the current leaf state at exit. Expected: State C");

    struct test_object *o = TEST_OBJECT(obj);

    o->tv_idx++;
    if (o->transition_bits != test_value[o->tv_idx])
    {
        printf("测试状态 C 退出失败\n");
    }
    // zassert_equal(o->transition_bits, test_value[o->tv_idx], "Test State C exit failed");

    if (o->terminate == EXIT)
    {
        smf_set_terminate(obj, -1);
        return;
    }

    o->transition_bits |= STATE_C_EXIT_BIT;
}

static void state_d_entry(void *obj)
{
    struct test_object *o = TEST_OBJECT(obj);

    o->tv_idx++;
}

static enum smf_state_result state_d_run(void *obj)
{
    /* Do nothing */
    return SMF_EVENT_HANDLED;
}

static void state_d_exit(void *obj)
{
    /* Do nothing */
}

static const struct smf_state test_states[] = {
    [STATE_A] = SMF_CREATE_STATE(state_a_entry, state_a_run, state_a_exit, NULL, NULL),
    [STATE_B] = SMF_CREATE_STATE(state_b_entry, state_b_run, state_b_exit, NULL, NULL),
    [STATE_C] = SMF_CREATE_STATE(state_c_entry, state_c_run, state_c_exit, NULL, NULL),
    [STATE_D] = SMF_CREATE_STATE(state_d_entry, state_d_run, state_d_exit, NULL, NULL),
};

void smf_test_handler(void)
{
    /* A) Test transitions */

    test_obj.transition_bits = 0;
    test_obj.terminate = NONE;
    smf_set_initial(SMF_CTX(&test_obj), &test_states[STATE_A]);

    for (int i = 0; i < SMF_RUN; i++)
    {
        if (smf_run_state(SMF_CTX(&test_obj)))
        {
            break;
        }
    }

    if (TEST_VALUE_NUM != test_obj.tv_idx)
    {
        printf("错误的测试值索引\n");
    }
    if (test_obj.transition_bits != test_value[test_obj.tv_idx])
    {
        printf("最终状态未达到\n");
    }

    // zassert_equal(TEST_VALUE_NUM, test_obj.tv_idx, "Incorrect test value index");
    // zassert_equal(test_obj.transition_bits, test_value[test_obj.tv_idx],
    //               "Final state not reached");

    /* B) Test termination in entry action */

    test_obj.transition_bits = 0;
    test_obj.terminate = ENTRY;
    smf_set_initial(SMF_CTX(&test_obj), &test_states[STATE_A]);

    for (int i = 0; i < SMF_RUN; i++)
    {
        if (smf_run_state(SMF_CTX(&test_obj)))
        {
            break;
        }
    }
    if (TEST_ENTRY_VALUE_NUM != test_obj.tv_idx)
    {
        printf("输入终止处的测试值索引不正确\n");
    }
    if (test_obj.transition_bits != test_value[test_obj.tv_idx])
    {
        printf("最终的条目终止状态尚未达成\n");
    }
    // zassert_equal(TEST_ENTRY_VALUE_NUM, test_obj.tv_idx,
    //               "Incorrect test value index for entry termination");
    // zassert_equal(test_obj.transition_bits, test_value[test_obj.tv_idx],
    //               "Final entry termination state not reached");

    /* C) Test termination in run action */

    test_obj.transition_bits = 0;
    test_obj.terminate = RUN;
    smf_set_initial(SMF_CTX(&test_obj), &test_states[STATE_A]);

    for (int i = 0; i < SMF_RUN; i++)
    {
        if (smf_run_state(SMF_CTX(&test_obj)))
        {
            break;
        }
    }
    if (TEST_RUN_VALUE_NUM != test_obj.tv_idx)
    {
        printf("运行终止时的测试值索引不正确\n");
    }
    if (test_obj.transition_bits != test_value[test_obj.tv_idx])
    {
        printf("最终运行终止状态未达成\n");
    }
    // zassert_equal(TEST_RUN_VALUE_NUM, test_obj.tv_idx,
    //               "Incorrect test value index for run termination");
    // zassert_equal(test_obj.transition_bits, test_value[test_obj.tv_idx],
    //               "Final run termination state not reached");

    /* D) Test termination in exit action */

    test_obj.transition_bits = 0;
    test_obj.terminate = EXIT;
    smf_set_initial(SMF_CTX(&test_obj), &test_states[STATE_A]);

    for (int i = 0; i < SMF_RUN; i++)
    {
        if (smf_run_state(SMF_CTX(&test_obj)))
        {
            break;
        }
    }
    if (TEST_EXIT_VALUE_NUM != test_obj.tv_idx)
    {
        printf("退出终止的测试值索引不正确\n");
    }
    if (test_obj.transition_bits != test_value[test_obj.tv_idx])
    {
        printf("最终的退出终止状态尚未达成\n");
    }
    // zassert_equal(TEST_EXIT_VALUE_NUM, test_obj.tv_idx,
    //               "Incorrect test value index for exit termination");
    // zassert_equal(test_obj.transition_bits, test_value[test_obj.tv_idx],
    //               "Final exit termination state not reached");
}