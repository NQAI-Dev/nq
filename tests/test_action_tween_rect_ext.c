#include <nq/action_tween_rect.h>
#include <nq/anim_rect.h>
#include "test_main.c"

static void test_action_tween_rect_ext(void) {
    NqAnimRect anim;
    nq_anim_rect_init(&anim, nq_rect(0, 0, 10, 10), nq_rect(10, 10, 20, 20), 1.0f, NQ_EASE_LINEAR);
    NqActionTweenRect *tween = nq_action_tween_rect_create(NULL, NULL);
    nq_action_tween_rect_set_anim(tween, &anim);
    NqAction *action = nq_action_tween_rect_action(tween);
    NQ_ASSERT(nq_action_state(action) == NQ_ACTION_IDLE);
    NqActionState state = nq_action_update(action, 0.5f);
    NQ_ASSERT(state == NQ_ACTION_RUNNING);
    NqRect current = nq_anim_rect_value(&anim);
    NQ_ASSERT(current.x == 5 && current.y == 5 && current.w == 15 && current.h == 15);
    state = nq_action_update(action, 0.5f);
    NQ_ASSERT(state == NQ_ACTION_FINISHED);
    current = nq_anim_rect_value(&anim);
    NQ_ASSERT(current.x == 10 && current.y == 10 && current.w == 20 && current.h == 20);
    nq_action_tween_rect_destroy(tween);
}
NQ_TEST_REGISTER("action_tween_rect", test_action_tween_rect_ext);
