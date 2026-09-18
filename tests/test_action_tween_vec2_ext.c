#include <nq/action_tween_vec2.h>
#include <nq/anim_vec2.h>
#include "test_main.c"

static void test_action_tween_vec2_ext(void) {
    NqAnimVec2 anim;
    nq_anim_vec2_init(&anim, nq_vec2f(0.0f, 0.0f), nq_vec2f(10.0f, 10.0f), 1.0f, NQ_EASE_LINEAR);
    NqActionTweenVec2 *tween = nq_action_tween_vec2_create(NULL, NULL);
    nq_action_tween_vec2_set_anim(tween, &anim);
    NqAction *action = nq_action_tween_vec2_action(tween);
    NQ_ASSERT(nq_action_state(action) == NQ_ACTION_IDLE);
    NqActionState state = nq_action_update(action, 0.5f);
    NQ_ASSERT(state == NQ_ACTION_RUNNING);
    NqVec2f current = nq_anim_vec2_value(&anim);
    NQ_ASSERT(current.x == 5.0f && current.y == 5.0f);
    state = nq_action_update(action, 0.5f);
    NQ_ASSERT(state == NQ_ACTION_FINISHED);
    current = nq_anim_vec2_value(&anim);
    NQ_ASSERT(current.x == 10.0f && current.y == 10.0f);
    nq_action_tween_vec2_destroy(tween);
}
NQ_TEST_REGISTER("action_tween_vec2", test_action_tween_vec2_ext);
